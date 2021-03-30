import os
import re
import xml.etree.ElementTree as ET
import argparse

# ==========================================================================================================
# CMD ARGUMENTS
parser = argparse.ArgumentParser(description='State machine code/diagram generator')
argsGenType = parser.add_mutually_exclusive_group(required=True)
argsGenType.add_argument('-code', action="store_true", help='generate C++ code based on hsmcpp library. '
                                                            'Supported arguments: '
                                                            '-class_name, -class_suffix, -template_hpp, -template_cpp, -dest_hpp, -dest_cpp, -dest_dir')
argsGenType.add_argument('-plantuml', action="store_true", help='generate plantuml state diagram. '
                                                                'Supported arguments: -out')

parser.add_argument('-scxml', '-s', type=str, required=True, help='path to state machine in SCXML format')
parser.add_argument('-class_name', '-c', type=str, help='class name used in generated code')
parser.add_argument('-class_suffix', '-cs', type=str, default="Base", help='suffix to append to class name (default: Base)')
parser.add_argument('-template_hpp', '-thpp', type=str, help='path to HPP template file')
parser.add_argument('-template_cpp', '-tcpp', type=str, help='path to CPP template file')
parser.add_argument('-dest_hpp', '-dhpp', type=str, help='path to file in which to store generated HPP content (default: ClassSuffixBase.hpp)')
parser.add_argument('-dest_cpp', '-dcpp', type=str, help='path to file in which to store generated CPP content (default: ClassSuffixBase.cpp)')
parser.add_argument('-dest_dir', '-d', type=str, help='path to folder where to store generated files (ignored if -dest_hpp and -dest_cpp are provided)')

parser.add_argument('-out', '-o', type=str, help='path for storing generated Plantuml file (only for -plantuml)')

args = parser.parse_args()

if args.code:
    if (args.scxml is None) or (args.class_name is None) or (args.template_hpp is None) or (args.template_cpp is None):
        print("ERROR: scxml, class_name, template_hpp or template_cpp was not provided")
        exit(1)
    if ((args.dest_hpp is None) and (args.dest_cpp is not None)) or ((args.dest_hpp is not None) and (args.dest_cpp is None)):
        print("ERROR: both dest_cpp and dest_hpp must be provided")
        exit(1)
    if (args.dest_hpp is None) and (args.dest_cpp is None) and (args.dest_dir is None):
        print("ERROR: destination was not provided")
        exit(1)
elif args.plantuml:
    if args.out is None:
        print("ERROR: -out option was not specified")
        exit(1)


# ==========================================================================================================
# FUNCTIONS
def validateIdentifier(identifier):
    isCorrect = False
    p = re.compile("^[a-zA-Z_]+[\w]*$")
    if p.match(identifier) is None:
        print(f"ERROR: <{identifier}> is not a valid C++ identifier (only digits, underscores, lowercase and uppercase Latin letters are permitted)")
        exit(3)
    else:
        isCorrect = True
    return isCorrect


def getTag(str):
    tag = str
    if "}" in tag:
        tag = str.partition("}")[2]
    return tag


def getCallbackName(elem):
    callback = None
    if elem is not None:
        if (getTag(elem.tag) == "onentry") or (getTag(elem.tag) == "onexit"):
            elemCallback = elem.find("{*}script")
            if (elemCallback is not None) and (elemCallback.text is not None):
                callback = elemCallback.text
            else:
                print(f"WARNING: skipping {getTag(elem.tag)} element because it doesn't have callback defined "
                      f"(<script> element is not defined or has empty value)\n{dumpXmlElement(elem)}")
        elif (getTag(elem.tag) == "invoke"):
            if "srcexpr" in elem.attrib:
                callback = elem.attrib["srcexpr"]
            else:
                print(f"WARNING: skipping {getTag(elem.tag)} element (srcexpr attribute is not defined)"
                      f"\n{dumpXmlElement(elem)}")
        elif getTag(elem.tag) == "script":
            if elem.text is not None:
                callback = elem.text
            else:
                print(f"WARNING: skipping {getTag(elem.tag)} element (doesnt have any value)\n{dumpXmlElement(elem)}")
        elif (getTag(elem.tag) == "transition") and ("cond" in elem.attrib):
            callback = elem.attrib["cond"]

    if callback is not None:
        validateIdentifier(callback)

    return callback


def dumpXmlElement(elem):
    return ET.tostring(elem).decode()


def parseScxmlStates(parentElement, rootDir, namePrefix):
    stateNodes = ["{*}state", "{*}include", "{*}final"]
    states = []
    initialState = None

    if "initial" in parentElement.attrib:
        initialState = namePrefix + parentElement.attrib["initial"]
    else:
        initialTransition = parentElement.find("{*}initial")
        if (initialTransition is not None):
            initialTransition = initialTransition.find("{*}transition")
            if (initialTransition is not None) and ("target" in initialTransition.attrib):
                initialState = namePrefix + initialTransition.attrib["target"]
            else:
                print(f"ERROR: <initial> element doesn't have transition defined or that transition "
                      f"doesnt have target (parent state [{parentElement.attrib['id']}])\n{dumpXmlElement(parentElement)}")
                exit(3)

    if 'src' in parentElement.attrib:
        includePath = os.path.join(rootDir, parentElement.attrib['src'])
        print(f"-- INCLUDE: {includePath}")
        subScxml = parseScxml(includePath, namePrefix + parentElement.attrib['id'] + "__")

        if subScxml is not None:
            states += subScxml
        else:
            print(f"ERROR: can't load included file: <{parentElement.attrib['src']}>")
            exit(4)

    for curNode in stateNodes:
        for curState in parentElement.findall(curNode):
            if getTag(curState.tag) == "include":
                if 'href' in curState.attrib:
                    newPrefix = namePrefix
                    includePath = os.path.join(rootDir, curState.attrib['href'])
                    print(f"-- INCLUDE: {includePath}")

                    # add prefix only if xi:include is wrapped inside aa state with no other items inside
                    if ('id' in parentElement.attrib) and (parentElement.find("{*}state") is None) and (len(parentElement.findall("{*}include")) == 1):
                        newPrefix += parentElement.attrib['id'] + "__"
                    subScxml = parseScxml(includePath, newPrefix)

                    if subScxml is not None:
                        states += subScxml
                    else:
                        print(f"ERROR: failed to load included file: [{includePath}]")
                        exit(4)
                else:
                    print(f"ERROR: include statement doesn't have href attribute defined\n{dumpXmlElement(curState)}")
                    exit(5)
            else:
                newState = {"id": namePrefix + curState.attrib["id"],
                            "transitions": []}
                validateIdentifier(newState["id"])

                if getTag(curState.tag) == "final":
                    newState["is_final"] = True
                elif newState['id'] == initialState:
                    newState["initial_state"] = True

                onentry = getCallbackName(curState.find("{*}onentry"))
                onexit = getCallbackName(curState.find("{*}onexit"))
                invoke = getCallbackName(curState.find("{*}invoke"))

                if onentry is not None:
                    newState["onentry"] = onentry
                if onexit is not None:
                    newState["onexit"] = onexit
                if invoke is not None:
                    newState["onstate"] = invoke

                for curTransition in curState.iterfind("{*}transition"):
                    if "event" in curTransition.attrib:
                        newTransition = {"event": curTransition.attrib["event"]}
                        validateIdentifier(newTransition["event"])

                        if "target" in curTransition.attrib:
                            newTransition["target"] = namePrefix + curTransition.attrib["target"]
                        else:
                            newTransition["target"] = namePrefix + newState["id"]

                        if "cond" in curTransition.attrib:
                            newTransition["condition"] = getCallbackName(curTransition)

                        transitionCallback = getCallbackName(curTransition.find("{*}script"))
                        if transitionCallback is not None:
                            newTransition["callback"] = transitionCallback

                        newState["transitions"].append(newTransition)
                    else:
                        print(f"WARNING: transition without event was skipped because it's not supported by hsmcpp\n{dumpXmlElement(curTransition)}")

                if (curState.find("{*}state") is not None) or (curState.find("{*}include") is not None) or ('src' in curState.attrib):
                    newState["states"] = parseScxmlStates(curState, rootDir, namePrefix)

                states.append(newState)
    return states


# NOTE: example of structure returned by parseScxml
# hsm = {"initial_state": "Off",
#        "states": [
#            {
#                "id": "Red",
#                "initial_state": "Yellow",
#                "onstate": "",
#                "onentry": "",
#                "onexit": "",
#                "transitions": [
#                    {
#                        "event": "NEXT_STATE",
#                        "target": "Yellow",
#                        "condition": "checkYellowTransition"
#                        "callback": "onTransition"
#                    }
#                ],
#                "states": [...]
#            }
#        ]}
def parseScxml(path, namePrefix = ""):
    hsm = None

    if os.path.exists(path):
        pathDir = os.path.dirname(path)
        tree = ET.parse(path)
        rootScxml = tree.getroot()

        # TODO: check if other namespaces are possible
        if ("{http://www.w3.org/2005/07/scxml}scxml" == rootScxml.tag) or ("scxml" == getTag(rootScxml.tag)):
            if "initial" in rootScxml.attrib:
                initialState = namePrefix + rootScxml.attrib["initial"]
            else:
                initialState = ""
            states = parseScxmlStates(rootScxml, pathDir, namePrefix)
            isCorrectInitialState = False

            for curState in states:
                if curState['id'] == initialState:
                    curState['initial_state'] = True
                    isCorrectInitialState = True
                    break

            if (isCorrectInitialState == True) or (len(initialState) == 0):
                hsm = states
            else:
                print(f"ERROR: incorrect initial state set for HSM: {initialState} [{path}]")
        else:
            print(f"ERROR: not an SCXML document: [{path}]")
    else:
        print(f"ERROR: file not found: [{path}]")
    return hsm


# ==========================================================================================================
# C++ code generation
def countOffset(line):
    offset = 0
    for c in line:
        if c != ' ':
            break
        offset += 1
    return offset


def generateOffset(offset):
    return ' ' * offset


def list2str(listData):
    result = ""
    for item in listData:
        result += item
    return result


def generateFile(genVars, templatePath, destPath):
    with open(templatePath, "r") as fileTemplate:
        with open(destPath, "w") as fileDestination:
            for templateLine in fileTemplate:
                for curVariable in genVars:
                    variableTypes = ["@", "%"]
                    for curType in variableTypes:
                        curVariableID = curType + curVariable + curType
                        if curVariableID in templateLine:
                            if curVariableID != templateLine.strip(' \n\r'):
                                # if line contains other text besides the keyword we insert data as-is
                                curVariableValue = list2str(genVars[curVariable])
                                if curType == "%":
                                    curVariableValue = curVariableValue.upper()
                                templateLine = templateLine.replace(curVariableID, curVariableValue)
                            else:
                                # if line contains only keyword we should offset data when inserting it
                                offset = generateOffset(countOffset(templateLine))
                                templateLine = ""
                                for curValue in genVars[curVariable]:
                                    if curType == "%":
                                        curValue = curValue.upper()
                                    templateLine += offset + curValue + "\n"

                fileDestination.writelines(templateLine)


def prepareHsmcppTransitionCallbackDeclaration(funcName):
    return f"virtual void {funcName}(const VariantList_t& args) = 0;"


def prepareHsmcppConditionCallbackDeclaration(funcName):
    return f"virtual bool {funcName}(const VariantList_t& args) = 0;"


def prepareHsmcppStateCallbackDeclaration(funcName):
    return f"virtual void {funcName}(const VariantList_t& args) = 0;"


def prepareHsmcppStateEnterCallbackDeclaration(funcName):
    return f"virtual bool {funcName}(const VariantList_t& args) = 0;"


def prepareHsmcppStateExitCallbackDeclaration(funcName):
    return f"virtual bool {funcName}() = 0;"


def generateCppCode(hsm, pathHpp, pathCpp):
    pendingStates = hsm

    genVars = {"ENUM_STATES_DEF": [],
               "ENUM_EVENTS_DEF": set(),
               "CLASS_NAME": args.class_name + args.class_suffix,
               "ENUM_STATES": f"{args.class_name}States",
               "ENUM_EVENTS": f"{args.class_name}Events",
               "INITIAL_STATE": "",
               "HSM_STATE_ACTIONS": set(),
               "HSM_STATE_ENTERING_ACTIONS": set(),
               "HSM_STATE_EXITING_ACTIONS": set(),
               "HSM_TRANSITION_ACTIONS": set(),
               "HSM_TRANSITION_CONDITIONS": set(),
               "HPP_FILE": os.path.basename(pathHpp),
               "REGISTER_STATES": [],
               "REGISTER_SUBSTATES": [],
               "REGISTER_TRANSITIONS": []}

    for curState in hsm:
        if ('initial_state' in curState) and (curState['initial_state'] == True):
            genVars['INITIAL_STATE'] = curState['id']
            break

    while len(pendingStates) > 0:
        substates = []
        for curState in pendingStates:
            if "states" in curState:
                substates += curState["states"]

                # initial state must be registered first
                for genEntryPoint in [True, False]:
                    for curSubstate in curState["states"]:
                        isInitialState = ("initial_state" in curSubstate) and (curSubstate["initial_state"] == True)
                        initialStateFlag = ""
                        print(f"{genEntryPoint}, {isInitialState}, {curSubstate['id']}")
                        if genEntryPoint == True:
                            if isInitialState == True:
                                initialStateFlag = ", true"
                            else:
                                continue
                        elif isInitialState == True:
                            continue

                        genVars["REGISTER_SUBSTATES"].append(f"registerSubstate({genVars['ENUM_STATES']}::{curState['id']}, " +
                                                                              f"{genVars['ENUM_STATES']}::{curSubstate['id']}" +
                                                                              f"{initialStateFlag});")

            genVars["ENUM_STATES_DEF"].append(f"{curState['id']},")
            registerCallbacks = ""

            if 'onstate' in curState:
                genVars["HSM_STATE_ACTIONS"].add(prepareHsmcppStateCallbackDeclaration(curState['onstate']))
                registerCallbacks += f", &{genVars['CLASS_NAME']}::{curState['onstate']}"
            else:
                registerCallbacks += ", nullptr"

            if 'onentry' in curState:
                genVars["HSM_STATE_ENTERING_ACTIONS"].add(prepareHsmcppStateEnterCallbackDeclaration(curState['onentry']))
                registerCallbacks += f", &{genVars['CLASS_NAME']}::{curState['onentry']}"
            else:
                registerCallbacks += ", nullptr"

            if 'onexit' in curState:
                genVars["HSM_STATE_EXITING_ACTIONS"].add(prepareHsmcppStateExitCallbackDeclaration(curState['onexit']))
                registerCallbacks += f", &{genVars['CLASS_NAME']}::{curState['onexit']}"
            else:
                registerCallbacks += ", nullptr"

            if "&" in registerCallbacks:
                registerCallbacks = ", this" + registerCallbacks
            else:
                registerCallbacks = ""

            genVars["REGISTER_STATES"].append(f"registerState<{genVars['CLASS_NAME']}>({genVars['ENUM_STATES']}::{curState['id']}{registerCallbacks});")

            for curTransition in curState["transitions"]:
                registerCallbacks = ""
                genVars["ENUM_EVENTS_DEF"].add(f"{curTransition['event']},")

                if 'callback' in curTransition:
                    genVars["HSM_TRANSITION_ACTIONS"].add(prepareHsmcppTransitionCallbackDeclaration(curTransition['callback']))
                    registerCallbacks += f", &{genVars['CLASS_NAME']}::{curTransition['callback']}"
                else:
                    registerCallbacks += ", nullptr"

                if 'condition' in curTransition:
                    genVars["HSM_TRANSITION_CONDITIONS"].add(prepareHsmcppConditionCallbackDeclaration(curTransition['condition']))
                    registerCallbacks += f", &{genVars['CLASS_NAME']}::{curTransition['condition']}"

                if "&" not in registerCallbacks:
                    registerCallbacks = ", this" + registerCallbacks
                else:
                    registerCallbacks = ""

                genVars["REGISTER_TRANSITIONS"].append(f"registerTransition<{genVars['CLASS_NAME']}>({genVars['ENUM_STATES']}::{curState['id']}, " +
                                                                                             f"{genVars['ENUM_STATES']}::{curTransition['target']}, " +
                                                                                             f"{genVars['ENUM_EVENTS']}::{curTransition['event']}" +
                                                                                             registerCallbacks + ");")
        pendingStates = substates

    generateFile(genVars, args.template_hpp, pathHpp)
    generateFile(genVars, args.template_cpp, pathCpp)


# ==========================================================================================================
# Plantuml generation
def generatePlantumlState(stateData, level):
    plantumlState = ""
    finalStates = []
    offset = generateOffset(level * 4)

    if 'initial_state' in stateData:
        plantumlState += f"{generateOffset(level * 4)}[*] --> {stateData['id']}\n"

    if "states" in stateData:
        plantumlState += f"\n{offset}state {stateData['id']} {{\n"

        for substate in stateData["states"]:
            (substatePlantuml, substateFinalStates) = generatePlantumlState(substate, level + 1)
            plantumlState += substatePlantuml
            finalStates += substateFinalStates
        plantumlState += f"{offset}}}\n\n"
    elif ("is_final" in stateData) and (stateData["is_final"] == True):
        finalStates = [stateData['id']]
    else:
        if "onentry" in stateData:
            plantumlState += f"\n{offset}{stateData['id']} : -> {stateData['onentry']}\n"
        if "onstate" in stateData:
            plantumlState += f"\n{offset}{stateData['id']} : {stateData['onstate']}\n"
        if "onexit" in stateData:
            plantumlState += f"\n{offset}{stateData['id']} : <- {stateData['onexit']}\n"

    for curTransition in stateData["transitions"]:
        plantumlState += f"{offset}{stateData['id']} --> {curTransition['target']}: {curTransition['event']}"
        if "condition" in curTransition:
            plantumlState += f"\\n[{curTransition['condition']}]"
        if "callback" in curTransition:
            plantumlState += f"\\n<{curTransition['callback']}>"
        plantumlState += "\n"

    return (plantumlState, finalStates)


def generatePlantuml(hsm, dest):
    plantuml = "@startuml\n\n"
    finalStates = []

    for curState in hsm:
        (statePlantuml, stateFinalStates) = generatePlantumlState(curState, 0)
        plantuml += statePlantuml
        finalStates += stateFinalStates

    for finalID in finalStates:
        plantuml = plantuml.replace(f"--> {finalID}", "--> [*]")

    plantuml += "\n@enduml"

    with open(dest, "w") as destFile:
        destFile.write(plantuml)


# ==========================================================================================================
# MAIN
if __name__ == "__main__":
    print(f"Loading [{args.scxml}] ...")
    hsm = parseScxml(args.scxml)

    if hsm is not None:
        if args.code:
            print("Generating code...")

            if args.dest_hpp is None:
                destHpp = f"{args.dest_dir}/{args.class_name}{args.class_suffix}.hpp"
            else:
                destHpp = args.dest_hpp
            if args.dest_cpp is None:
                destCpp = f"{args.dest_dir}/{args.class_name}{args.class_suffix}.cpp"
            else:
                destCpp = args.dest_cpp

            generateCppCode(hsm, destHpp, destCpp)
        elif args.plantuml:
            print(f"Generating PlantUML...")
            generatePlantuml(hsm, args.out)
    else:
        print(f"ERROR: failed to parse SCXML: [{args.scxml}]")
        exit(2)