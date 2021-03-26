import os
import re
import xml.etree.ElementTree as ET
import argparse

# ==========================================================================================================
# CMD ARGUMENTS
parser = argparse.ArgumentParser(description='State machine code/diagram generator')
argsGenType = parser.add_mutually_exclusive_group(required=True)
argsGenType.add_argument('-code', action="store_true", help='generate C++ code based on hsmcpp library')
argsGenType.add_argument('-plantuml', action="store_true", help='generate plantuml state diagram')

parser.add_argument('-scxml', '-s', type=str, required=True, help='path to state machine in SCXML format')
parser.add_argument('-class_name', '-c', type=str, help='class name used in generated code')
parser.add_argument('-class_suffix', '-cs', type=str, default="Base", help='suffix to append to class name (default: Base)')
parser.add_argument('-template_hpp', '-thpp', type=str, help='path to HPP template file')
parser.add_argument('-template_cpp', '-tcpp', type=str, help='path to CPP template file')
parser.add_argument('-dest_hpp', '-dhpp', type=str, help='path to file in which to store generated HPP content (default: ClassSuffixBase.hpp)')
parser.add_argument('-dest_cpp', '-dcpp', type=str, help='path to file in which to store generated CPP content (default: ClassSuffixBase.cpp)')
parser.add_argument('-dest', '-d', type=str, help='path to folder where to store generated files (ignored if -dest_hpp and -dest_cpp are provided)')

args = parser.parse_args()

if args.code:
    if (args.scxml == None) or (args.class_name == None) or (args.template_hpp == None) or (args.template_cpp == None):
        print("ERROR: scxml, class_name, template_hpp or template_cpp was not provided")
        exit(1)
    if ((args.dest_hpp == None) and (args.dest_cpp != None)) or ((args.dest_hpp != None) and (args.dest_cpp == None)):
        print("ERROR: both dest_cpp and dest_hpp must be provided")
        exit(1)
    if (args.dest_hpp == None) and (args.dest_cpp == None) and (args.dest == None):
        print("ERROR: destination was not provided")
        exit(1)


# ==========================================================================================================
# FUNCTIONS
def validateIdentifier(identifier):
    isCorrect = False
    p = re.compile("^[a-zA-Z_]+[\w]*$")
    if p.match(identifier) == None:
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
    if elem != None:
        if (getTag(elem.tag) == "onentry") or (getTag(elem.tag) == "onexit"):
            elemCallback = elem.find("{*}script")
            if elemCallback != None:
                callback = elemCallback.attrib["src"]
            else:
                print(f"WARNING: skipping {getTag(elem.tag)} element because it doesn't have callback defined (<script> element was expected)")
        elif (getTag(elem.tag) == "invoke") or (getTag(elem.tag) == "script"):
            if "src" in elem.attrib:
                callback = elem.attrib["src"]
            else:
                print(f"WARNING: skipping {getTag(elem.tag)} element (src attribute is not defined)")
        elif (getTag(elem.tag) == "transition") and ("cond" in elem.attrib):
            callback = elem.attrib["cond"]

    if callback != None:
        validateIdentifier(callback)

    return callback


def parseScxmlStates(parentElement):
    stateNodes = ["{*}state", "{*}final"]
    states = []

    for curNode in stateNodes:
        for curState in parentElement.findall(curNode):
            newState = {"id": curState.attrib["id"],
                        "transitions": []}
            validateIdentifier(newState["id"])

            if curState.find("{*}state") != None:
                if "initial" in curState.attrib:
                    newState["initial_state"] = curState.attrib["initial"]
                else:
                    initialTransition = curState.find("{*}transition")
                    if (initialTransition != None) and ("target" in initialTransition.attrib):
                        newState["initial_state"] = initialTransition.attrib["target"]
                    else:
                        print(f"ERROR: <initial> element doesn't have transition defined or that transition doesnt have target (parent state [{newState['id']}])")
                        exit(3)

            onentry = getCallbackName(curState.find("{*}onentry"))
            onexit = getCallbackName(curState.find("{*}onexit"))
            invoke = getCallbackName(curState.find("{*}invoke"))

            if onentry != None:
                newState["onentry"] = onentry
            if onexit != None:
                newState["onexit"] = onexit
            if invoke != None:
                newState["onstate"] = invoke

            for curTransition in curState.iterfind("{*}transition"):
                if "event" in curTransition.attrib:
                    newTransition = {"event": curTransition.attrib["event"]}
                    validateIdentifier(newTransition["event"])

                    if "target" in curTransition.attrib:
                        newTransition["target"] = curTransition.attrib["target"]
                    else:
                        newTransition["target"] = newState["id"]

                    if "cond" in curTransition.attrib:
                        newTransition["condition"] = getCallbackName(curTransition)

                    transitionCallback = getCallbackName(curTransition.find("{*}script"))
                    if transitionCallback != None:
                        newTransition["callback"] = transitionCallback

                    newState["transitions"].append(newTransition)
                else:
                    print("WARNING: transition without event was skipped because it's not supported by hsmcpp")

            if curState.find("{*}state") != None:
                newState["states"] = parseScxmlStates(curState)

            states.append(newState)
    return states


# NOTE: example of structure returned by parseScxml
# hsm = {"initial_state": "Off",
#        "states_list": ["", "", ...]
#        "states": [
#            {
#                "id": "Red",
#                "initial_state": "Yellow",
#                "onstate": "",
#                "onenter": "",
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
def parseScxml(path):
    hsm = None
    tree = ET.parse(path)
    rootScxml = tree.getroot()

    # TODO: check if other namespaces are possible
    if ("{http://www.w3.org/2005/07/scxml}scxml" == rootScxml.tag) or ("scxml" == getTag(rootScxml.tag)):
        if "initial" in rootScxml.attrib:
            initialState = rootScxml.attrib["initial"]
            states = parseScxmlStates(rootScxml)

            hsm = {"initial_state": initialState, "states": states}
        else:
            print("ERROR: initial state is not set or document is empty")
    else:
        print("ERROR: not an SCXML document")
    return hsm


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


def generateFile(vars, templatePath, destPath):
    with open(templatePath, "r") as fileTemplate:
        with open(destPath, "w") as fileDestination:
            for templateLine in fileTemplate:
                for curVariable in vars:
                    curVariableID = "@" + curVariable + "@"
                    if curVariableID in templateLine:
                        if curVariableID != templateLine.strip(' \n\r'):
                            # if line contains other text besides the keywork we insert data as-is
                            templateLine = templateLine.replace(curVariableID, list2str(vars[curVariable]))
                        else:
                            # if line contains only keyword we should offset data when inserting it
                            offset = generateOffset(countOffset(templateLine))
                            templateLine = ""
                            for curValue in vars[curVariable]:
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
    pendingStates = hsm["states"]

    vars = {"ENUM_STATES_DEF": [],
            "ENUM_EVENTS_DEF": set(),
            "CLASS_NAME": args.class_name + "Base",
            "ENUM_STATES": f"{args.class_name}States",
            "ENUM_EVENTS": f"{args.class_name}Events",
            "INITIAL_STATE": hsm["initial_state"],
            "HSM_STATE_ACTIONS": set(),
            "HSM_STATE_ENTERING_ACTIONS": set(),
            "HSM_STATE_EXITING_ACTIONS": set(),
            "HSM_TRANSITION_ACTIONS": set(),
            "HSM_TRANSITION_CONDITIONS": set(),
            "HPP_FILE": os.path.basename(pathHpp),
            "REGISTER_STATES": [],
            "REGISTER_SUBSTATES": [],
            "REGISTER_TRANSITIONS": []}

    while len(pendingStates) > 0:
        substates = []
        for curState in pendingStates:
            if "states" in curState:
                substates += curState["states"]
                # initial state must be registered first
                vars["REGISTER_SUBSTATES"].append(f"registerSubstate({vars['ENUM_STATES']}::{curState['id']}, " +
                                                  f"{vars['ENUM_STATES']}::{curState['initial_state']}" +
                                                  ", true);")

                for curSubstate in curState["states"]:
                    isInitialState = ""
                    if curState["initial_state"] == curSubstate["id"]:
                        continue
                    vars["REGISTER_SUBSTATES"].append(f"registerSubstate({vars['ENUM_STATES']}::{curState['id']}, " +
                                                                       f"{vars['ENUM_STATES']}::{curSubstate['id']}" +
                                                                       f"{isInitialState});")

            vars["ENUM_STATES_DEF"].append(f"{curState['id']},")
            registerCallbacks = ""

            if 'onstate' in curState:
                vars["HSM_STATE_ACTIONS"].add(prepareHsmcppStateCallbackDeclaration(curState['onstate']))
                registerCallbacks += f", &{vars['CLASS_NAME']}::{curState['onstate']}"
            else:
                registerCallbacks += ", nullptr"

            if 'onentry' in curState:
                vars["HSM_STATE_ENTERING_ACTIONS"].add(prepareHsmcppStateEnterCallbackDeclaration(curState['onentry']))
                registerCallbacks += f", &{vars['CLASS_NAME']}::{curState['onentry']}"
            else:
                registerCallbacks += ", nullptr"

            if 'onexit' in curState:
                vars["HSM_STATE_EXITING_ACTIONS"].add(prepareHsmcppStateExitCallbackDeclaration(curState['onexit']))
                registerCallbacks += f", &{vars['CLASS_NAME']}::{curState['onexit']}"
            else:
                registerCallbacks += ", nullptr"

            if "&" in registerCallbacks:
                registerCallbacks = ", this" + registerCallbacks
            else:
                registerCallbacks = ""

            vars["REGISTER_STATES"].append(f"registerState<{vars['CLASS_NAME']}>({vars['ENUM_STATES']}::{curState['id']}{registerCallbacks});")

            for curTransition in curState["transitions"]:
                registerCallbacks = ""
                vars["ENUM_EVENTS_DEF"].add(f"{curTransition['event']},")

                if 'callback' in curTransition:
                    vars["HSM_TRANSITION_ACTIONS"].add(prepareHsmcppTransitionCallbackDeclaration(curTransition['callback']))
                    registerCallbacks += f", &{vars['CLASS_NAME']}::{curTransition['callback']}"
                else:
                    registerCallbacks += ", nullptr"

                if 'condition' in curTransition:
                    vars["HSM_TRANSITION_CONDITIONS"].add(prepareHsmcppConditionCallbackDeclaration(curTransition['condition']))
                    registerCallbacks += f", &{vars['CLASS_NAME']}::{curTransition['condition']}"

                if "&" not in registerCallbacks:
                    registerCallbacks = ", this" + registerCallbacks
                else:
                    registerCallbacks = ""

                vars["REGISTER_TRANSITIONS"].append(f"registerTransition<{vars['CLASS_NAME']}>({vars['ENUM_STATES']}::{curState['id']}, " +
                                                                                             f"{vars['ENUM_STATES']}::{curTransition['target']}, " +
                                                                                             f"{vars['ENUM_EVENTS']}::{curTransition['event']}" +
                                                                                             registerCallbacks + ");")
        pendingStates = substates

    generateFile(vars, args.template_hpp, pathHpp)
    generateFile(vars, args.template_cpp, pathCpp)


# ==========================================================================================================
# MAIN
if __name__ == "__main__":
    hsm = parseScxml(args.scxml)

    if hsm != None:
        if args.code:
            print("Generating code...")

            if args.dest_hpp == None:
                destHpp = f"{args.dest}/{args.class_name}{args.class_suffix}.hpp"
            else:
                destHpp = args.dest_hpp
            if args.dest_cpp == None:
                destCpp = f"{args.dest}/{args.class_name}{args.class_suffix}.cpp"
            else:
                destCpp = args.dest_cpp

            generateCppCode(hsm, destHpp, destCpp)
        elif args.plantuml:
            print("TODO: Plantuml generation is not implemented yet")
            exit(7)
    else:
        print("ERROR: failed to parse SCXML")
        exit(2)