import os
import re
import xml.etree.ElementTree as ET
import argparse

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
            elemCallback = xmlFind(elem, "script")
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


# NOTE: find, findall don't work correctly in versions prior to 3.8 (https://bugs.python.org/issue28238)
def xmlFind(node, tag):
    res = None
    for child in node:
        if getTag(child.tag) == tag:
            res = child
            break
    return res
    

def xmlFindAll(node, tag):
    for child in node:
        if getTag(child.tag) == tag:
            yield child


def parseScxmlStates(parentElement, rootDir, namePrefix):
    nodesWithChildren = ["state", "parallel", "include"]
    stateNodes = nodesWithChildren + ["final", "history"]
    states = []
    initialStates = []
    initialStatesCondition = {}

    # entry point can be defined in multiple ways:
    #  - as "initial" attribute of <state> node
    #  - as <initial> with node with transitions inside of <state>
    if "initial" in parentElement.attrib:
        initialStates = [namePrefix + parentElement.attrib["initial"]]
    else:
        initialTransition = xmlFind(parentElement, "initial")
        if (initialTransition is not None):
            for initialTransition in xmlFindAll(initialTransition, "transition"):
                if "target" in initialTransition.attrib:
                    initialStateName = namePrefix + initialTransition.attrib["target"]
                    initialStates.append(initialStateName)
                    if "event" in initialTransition.attrib:
                        initialStatesCondition[initialStateName] = initialTransition.attrib["event"]

            if len(initialStates) == 0:
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
        for curState in xmlFindAll(parentElement, curNode):
            if getTag(curState.tag) in stateNodes:
                if getTag(curState.tag) == "include":
                    if 'href' in curState.attrib:
                        newPrefix = namePrefix
                        includePath = os.path.join(rootDir, curState.attrib['href'])
                        print(f"-- INCLUDE: {includePath}")

                        # add prefix only if xi:include is wrapped inside aa state with no other items inside
                        if ('id' in parentElement.attrib) and (xmlFind(parentElement, "state") is None) and (len(xmlFindAll(parentElement, "include")) == 1):
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
                                "is_history": False,
                                "transitions": []}
                    validateIdentifier(newState["id"])

                    if getTag(curState.tag) == "final":
                        newState["is_final"] = True

                    if getTag(curState.tag) == "history":
                        newState["is_history"] = True
                        newState["history_type"] = curState.attrib["type"]

                    if (getTag(parentElement.tag) == "parallel") or (newState['id'] in initialStates):
                        newState["initial_state"] = True
                        if newState['id'] in initialStatesCondition:
                            newState["initial_state_condition"] = initialStatesCondition[newState['id']]

                    onentry = getCallbackName(xmlFind(curState, "onentry"))
                    onexit = getCallbackName(xmlFind(curState, "onexit"))
                    invoke = getCallbackName(xmlFind(curState, "invoke"))

                    if onentry is not None:
                        newState["onentry"] = onentry
                    if onexit is not None:
                        newState["onexit"] = onexit
                    if invoke is not None:
                        newState["onstate"] = invoke

                    for curTransition in xmlFindAll(curState, "transition"):
                        if "event" in curTransition.attrib:
                            newTransition = {"event": curTransition.attrib["event"]}
                            validateIdentifier(newTransition["event"])

                            if "target" in curTransition.attrib:
                                newTransition["target"] = namePrefix + curTransition.attrib["target"]
                            else:
                                newTransition["target"] = namePrefix + newState["id"]

                            if "cond" in curTransition.attrib:
                                newTransition["condition"] = getCallbackName(curTransition)

                            transitionCallback = getCallbackName(xmlFind(curTransition, "script"))
                            if transitionCallback is not None:
                                newTransition["callback"] = transitionCallback

                            newState["transitions"].append(newTransition)
                        else:
                            print(f"WARNING: transition without event was skipped because it's not supported by hsmcpp\n{dumpXmlElement(curTransition)}")

                    for nodeName in nodesWithChildren:
                        if (xmlFind(curState, nodeName) is not None) or ('src' in curState.attrib):
                            newState["states"] = parseScxmlStates(curState, rootDir, namePrefix)
                            break
                    states.append(newState)
    return states


# NOTE: example of structure returned by parseScxml
# hsm = {"initial_state": "Off",
#        "states": [
#            {
#                "id": "Red",
#                "initial_state": "Yellow",
#                "initial_state_condition": "event_1",
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


def applySingleVariableToLine(line, variableName, variableValue):
    newLine = line
    for curType in ["@", "%"]:
        curVariableID = curType + variableName + curType
        if curVariableID in newLine:
            if curVariableID != newLine.strip(' \n\r'):
                # if line contains other text besides the keyword we insert data as-is
                curVariableValue = list2str(variableValue)
                if curType == "%":
                    curVariableValue = curVariableValue.upper()
                newLine = newLine.replace(curVariableID, curVariableValue)
            else:
                # if line contains only keyword we should offset data when inserting it
                offset = generateOffset(countOffset(newLine))
                newLine = ""
                for curValue in variableValue:
                    if curType == "%":
                        curValue = curValue.upper()
                    newLine += offset + curValue + "\n"
    return newLine

def applyGenVariablesToLine(genVars, line):
    newLine = line

    for curVariable in genVars:
        newLine = applySingleVariableToLine(newLine, curVariable, genVars[curVariable])

    return newLine


def generateFile(genVars, templatePath, destPath):
    with open(templatePath, "r") as fileTemplate:
        with open(destPath, "w") as fileDestination:
            currentBlock = None
            currentBlockVariable = None

            for templateLine in fileTemplate:
                # ~~~BLOCK:variable~~~
                # ... repeat this code for each variable value ...
                # ~~~BLOCK:END~~~
                if templateLine.startswith("~~~BLOCK:"):
                    currentBlockVariable = templateLine[len("~~~BLOCK:"):][:-4]
                    if currentBlockVariable in genVars:
                        currentBlock = []
                    else:
                        print(f"ERROR: BLOCK variable <{currentBlockVariable}> is not supported")
                        exit(6)
                elif templateLine.startswith("~~~BLOCK_END~~~"):
                    if currentBlockVariable:
                        templateLine = ""
                        for curVariableValue in genVars[currentBlockVariable]:
                            for blockLine in currentBlock:
                                blockLine = applySingleVariableToLine(blockLine, currentBlockVariable, curVariableValue)
                                templateLine += applyGenVariablesToLine(genVars, blockLine)

                        currentBlock = None
                        currentBlockVariable = None
                        fileDestination.writelines(templateLine)
                    else:
                        print("ERROR: BLOCK_END was found, but there is no BLOCK BEGIN")
                        exit(6)
                elif currentBlockVariable:
                    currentBlock.append(templateLine)
                else:
                    templateLine = applyGenVariablesToLine(genVars, templateLine)
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

    genVars = {"ENUM_STATES_ITEM": [],
               "ENUM_EVENTS_ITEM": set(),
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
               "REGISTER_TRANSITIONS": [],
               "ENUM_STATES_GETNAME_CASES": [],
               "ENUM_EVENTS_GETNAME_CASES": []}

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
                        if (genEntryPoint is False) and (curSubstate["is_history"] is True):
                            print(f"FOUND HISTORY: {curSubstate}")
                            historyType = ""
                            defaultTarget = ""
                            historyCallback = ""

                            if "history_type" in curSubstate:
                                if curSubstate["history_type"] == "deep":
                                    historyType = "HistoryType::DEEP"
                                else:
                                    historyType = "HistoryType::SHALLOW"

                            if ('transitions' in curSubstate) and (len(curSubstate['transitions']) > 0):
                                defaultTransition = curSubstate['transitions'][0]
                                defaultTarget = f", {genVars['ENUM_STATES']}::{defaultTransition['target']}"
                                if 'callback' in defaultTransition:
                                    genVars["HSM_TRANSITION_ACTIONS"].add(prepareHsmcppTransitionCallbackDeclaration(defaultTransition['callback']))
                                    historyCallback += f", &{genVars['CLASS_NAME']}::{defaultTransition['callback']}"

                            genVars["REGISTER_SUBSTATES"].append(
                                                    f"registerHistory({genVars['ENUM_STATES']}::{curState['id']}, " +
                                                    f"{genVars['ENUM_STATES']}::{curSubstate['id']}, " +
                                                    f"{historyType}" +
                                                    f"{defaultTarget}"
                                                    f"{historyCallback});")
                        else:
                            registerSubstateFunc = "registerSubstate"
                            isInitialState = ("initial_state" in curSubstate) and (curSubstate["initial_state"] == True)
                            initialStateCondition = ""

                            if genEntryPoint == True:
                                if isInitialState == True:
                                    registerSubstateFunc = "registerSubstateEntryPoint"
                                    if "initial_state_condition" in curSubstate:
                                        initialStateCondition = f", {genVars['ENUM_EVENTS']}::{curSubstate['initial_state_condition']}"
                                else:
                                    continue
                            elif isInitialState == True:
                                continue

                            genVars["REGISTER_SUBSTATES"].append(f"{registerSubstateFunc}({genVars['ENUM_STATES']}::{curState['id']}, " +
                                                                                        f"{genVars['ENUM_STATES']}::{curSubstate['id']}" +
                                                                                        f"{initialStateCondition});")

            genVars["ENUM_STATES_ITEM"].append({curState['id']})
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

            if curState['is_history'] is False:
                genVars["REGISTER_STATES"].append(f"registerState<{genVars['CLASS_NAME']}>({genVars['ENUM_STATES']}::{curState['id']}{registerCallbacks});")

                for curTransition in curState["transitions"]:
                    registerCallbacks = ""
                    genVars["ENUM_EVENTS_ITEM"].add(curTransition['event'])

                    if 'callback' in curTransition:
                        genVars["HSM_TRANSITION_ACTIONS"].add(prepareHsmcppTransitionCallbackDeclaration(curTransition['callback']))
                        registerCallbacks += f", &{genVars['CLASS_NAME']}::{curTransition['callback']}"
                    else:
                        registerCallbacks += ", nullptr"

                    if 'condition' in curTransition:
                        genVars["HSM_TRANSITION_CONDITIONS"].add(prepareHsmcppConditionCallbackDeclaration(curTransition['condition']))
                        registerCallbacks += f", &{genVars['CLASS_NAME']}::{curTransition['condition']}"

                    if "&" in registerCallbacks:
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
def getHistoryStateName(state):
    stateName = ""
    if state['is_history'] is True:
        if ('history_type' in state) and (state['history_type'] == 'deep'):
            stateName = "[H*]"
        else:
            stateName = "[H]"
    return stateName


def findState(topLevelState, stateId):
    resultState = None

    for curState in topLevelState:
        if stateId == curState['id']:
            resultState = curState
            break
        elif 'states' in curState:
            resultState = findState(curState['states'], stateId)

        if resultState is not None:
            break
    return resultState


def findTranstitions(topLevelState, targetStateId):
    transitions = {}

    for curState in topLevelState:
        if 'transitions' in curState:
            for curTransition in curState['transitions']:
                if curTransition['target'] == targetStateId:
                    if curState['id'] not in transitions:
                        transitions[curState['id']] = []
                    transitions[curState['id']].append(curTransition)
        if 'states' in curState:
            transitions = {**transitions, **findTranstitions(curState['states'], targetStateId)}
    return transitions


def isHistoryState(topLevelState, stateId):
    state = findState(topLevelState, stateId)
    if state is not None:
        is_history = state['is_history']
    else:
        is_history = False

    return is_history


def generateStatesList(states, level):
    statesList = ""
    offset = generateOffset(level * 4)

    for curState in states:
        if curState['is_history'] is False:
            statesList += f"{offset}state {curState['id']}\n"
    statesList += "\n"
    return statesList


def prepareTransitionHighlight(highlight, highlightColorActive, stateData, curTransition):
    highlightTransition = ""
    highlightTransitionEvent = ""
    transitionArgs = ""

    if highlight and curTransition['target'] in highlight['transitions']:
        highlightInfo = highlight['transitions'][curTransition['target']]
        if highlightInfo["from"] == stateData['id']:
            if (stateData['is_history'] is True) or (highlightInfo["event"] == curTransition['event']):
                highlightTransition = f"[#{highlightColorActive},bold]"
                highlightTransitionEvent = "**"
                if 'args' in highlightInfo:
                    transitionArgs = highlightInfo['args']

    return (highlightTransition, highlightTransitionEvent, transitionArgs)


def preparePlantumlTransition(offset, fromState, toState, curTransition, highlightTransition, highlightTransitionEvent):
    plantumlTransition = f"{offset}{fromState} -{highlightTransition}-> {toState}: {highlightTransitionEvent}{curTransition['event']}{highlightTransitionEvent}"

    if "condition" in curTransition:
        plantumlTransition += f"\\n{highlightTransitionEvent}[{curTransition['condition']}]{highlightTransitionEvent}"
    if "callback" in curTransition:
        plantumlTransition += f"\\n{highlightTransitionEvent}<{curTransition['callback']}>{highlightTransitionEvent}"
    plantumlTransition += "\n"
    return plantumlTransition


# hightlight = {"style": {"active_state": "00FF00", "blocked_transition": "FFAA00"},
#               "active_states": ["state_3", ...],
#               "callback": {"state_1": {"onstate|onexit|onentry": False, "args": [...]}, ...},
#               "transitions": {"parent_2": {"from": "parent_1", "event": "event_next_parent", "args": [...]}, # regular
#                               "state_4": {"from": "state_4", "event": "event_self", "args": [...]},          # self
#                               "state_3": {"from": "", "event": "event_self", "args": [...]}, ...}}           # entry
def generatePlantumlState(hsm, stateData, level, highlight=None):
    plantumlState = ""
    finalStates = []
    offset = generateOffset(level * 4)

    # -----------------------------------
    # Configure colors
    highlightColorActive = None
    highlightColorBlocked = None

    if (highlight is not None) and ('style' in highlight):
        if 'active_state' in highlight['style']:
            highlightColorActive = highlight['style']['active_state']
        if 'blocked_transition' in highlight['style']:
            highlightColorBlocked = highlight['style']['blocked_transition']
    if highlightColorActive is None:
        highlightColorActive = "00FF00"
    if highlightColorBlocked is None:
        highlightColorBlocked = "FFAA00"

    # -----------------------------------
    # handle initial states
    if 'initial_state' in stateData:
        condition = ""
        highlightEntryTransition = ""
        entryTransitionArgs = ""
        if 'initial_state_condition' in stateData:
            condition = f" : {stateData['initial_state_condition']}"

        if highlight and (stateData['id'] in highlight['transitions']):
            highlightEntryTransitionInfo = highlight['transitions'][stateData['id']]
            if (highlightEntryTransitionInfo["from"] == "") and\
               (('initial_state_condition' not in stateData) or\
                highlightEntryTransitionInfo["event"] == stateData['initial_state_condition']):
                    highlightEntryTransition = f"[#{highlightColorActive},bold]"
                    if "args" in highlightEntryTransitionInfo:
                        entryTransitionArgs = highlightEntryTransitionInfo['args']

        plantumlState += f"{generateOffset(level * 4)}[*] -{highlightEntryTransition}-> {stateData['id']}{condition}\n"
        if entryTransitionArgs and len(entryTransitionArgs) > 0:
            plantumlState += f"{offset}note on link\n{offset}  **Arguments:**\n\n{offset}  {entryTransitionArgs}\n{offset}end note\n"

    # -----------------------------------
    # handle substates
    if "states" in stateData:
        plantumlState += f"\n{offset}state {stateData['id']} {{\n"
        plantumlState += generateStatesList(stateData['states'], level + 1)

        for substate in stateData["states"]:
            (substatePlantuml, substateFinalStates) = generatePlantumlState(hsm, substate, level + 1, highlight)
            plantumlState += substatePlantuml
            finalStates += substateFinalStates
        plantumlState += f"{offset}}}\n\n"
    elif ("is_final" in stateData) and (stateData["is_final"] == True):
        finalStates = [stateData['id']]
    else:
        highlightOnEntry = ""
        highlightOnExit = ""
        highlightOnState = ""
        highlightState = False
        callbackFailedMsg = ""
        callbackArgs = ""

        if highlight and stateData['id'] in highlight['active_states']:
            highlightState = True
        if highlight and stateData['id'] in highlight['callback']:
            highlightState = True
            stateHighlightInfo = highlight['callback'][stateData['id']]
            if "onentry" in stateHighlightInfo:
                highlightOnEntry = "**"
                if stateHighlightInfo["onentry"] is False:
                    callbackFailedMsg = stateData['onentry']
            if "onexit" in stateHighlightInfo:
                highlightOnExit = "**"
                if stateHighlightInfo["onexit"] is False:
                    callbackFailedMsg = stateData['onexit']
            if "onstate" in stateHighlightInfo:
                highlightOnState = "**"
                if stateHighlightInfo["onstate"] is False:
                    callbackFailedMsg = stateData['onstate']
            if 'args' in stateHighlightInfo:
                callbackArgs = stateHighlightInfo['args']

        if "onentry" in stateData:
            plantumlState += f"\n{offset}{stateData['id']} : {highlightOnEntry}-> {stateData['onentry']}{highlightOnEntry}\n"
        if "onstate" in stateData:
            plantumlState += f"\n{offset}{stateData['id']} : {highlightOnState}{stateData['onstate']}{highlightOnState}\n"
        if "onexit" in stateData:
            plantumlState += f"\n{offset}{stateData['id']} : {highlightOnExit}<- {stateData['onexit']}{highlightOnExit}\n"

        if highlightState and (stateData['is_history'] is False):
            if len(callbackFailedMsg) > 0:
                plantumlState += f"{offset}state \"**{stateData['id']}**\" as {stateData['id']} #{highlightColorBlocked}\n"
                plantumlState += f"note left of {stateData['id']} : "\
                                 f"**Transition blocked**\\n{callbackFailedMsg}() returned FALSE"
                if len(callbackArgs) > 0:
                    plantumlState += f"\\n\\n**Arguments**\\n\\n{callbackArgs}"
                plantumlState += "\n"
            else:
                plantumlState += f"{offset}state \"**{stateData['id']}**\" as {stateData['id']} #{highlightColorActive}\n"
                if callbackArgs and len(callbackArgs) > 0:
                    plantumlState += f"note left of {stateData['id']} : **Arguments**\\n\\n{callbackArgs}\n"

    # highlighting transition targets must be done AFTER state is fully defined (since transitions can appear earlier)
    # but skip self transitions since in this case state will be active
    if highlight \
       and (stateData['is_history'] is False) \
       and (stateData['id'] in highlight['transitions']) \
       and (stateData['id'] != highlight['transitions'][stateData['id']]["from"]):
        plantumlState += f"{offset}state {stateData['id']} ##[dotted]{highlightColorActive}\n"

    # -----------------------------------
    # handle transitions
    if stateData['is_history'] is True:
        historyTransitions = findTranstitions(hsm, stateData['id'])

        # render incomming history transitions
        for fromState in historyTransitions:
            for curTransition in historyTransitions[fromState]:
                (highlightTransition, highlightTransitionEvent, transitionArgs) = prepareTransitionHighlight(highlight,
                                                                                                             highlightColorActive,
                                                                                                             findState(hsm, fromState),
                                                                                                             curTransition)
                plantumlState += preparePlantumlTransition(offset, fromState, getHistoryStateName(stateData),
                                                           curTransition, highlightTransition, highlightTransitionEvent)

    for curTransition in stateData["transitions"]:
        (highlightTransition, highlightTransitionEvent, transitionArgs) = prepareTransitionHighlight(highlight,
                                                                                                     highlightColorActive,
                                                                                                     stateData,
                                                                                                     curTransition)
        # render transition
        if stateData['is_history'] is True:
            plantumlState += preparePlantumlTransition(offset, getHistoryStateName(stateData), curTransition['target'],
                                                       curTransition, highlightTransition, highlightTransitionEvent)
        elif isHistoryState(hsm, curTransition['target']) is False:
            plantumlState += preparePlantumlTransition(offset, stateData['id'], curTransition['target'],
                                                       curTransition, highlightTransition, highlightTransitionEvent)

        if transitionArgs and len(transitionArgs) > 0:
            plantumlState += f"{offset}note on link\n{offset}  **Arguments:**\n\n{offset}  {transitionArgs}\n{offset}end note\n"
    return (plantumlState, finalStates)


def generatePlantumlInMemory(hsm, highlight=None):
    plantuml = "@startuml\n\n"
    finalStates = []

    plantuml += generateStatesList(hsm, 0)

    for curState in hsm:
        (statePlantuml, stateFinalStates) = generatePlantumlState(hsm, curState, 0, highlight)
        plantuml += statePlantuml
        finalStates += stateFinalStates

    for finalID in finalStates:
        plantuml = plantuml.replace(f"--> {finalID}", "--> [*]")

    plantuml += "\n@enduml"
    return plantuml


def generatePlantuml(hsm, dest, highlight=None):
    plantuml = generatePlantumlInMemory(hsm, highlight)

    with open(dest, "w") as destFile:
        destFile.write(plantuml)


# ==========================================================================================================
# MAIN
if __name__ == "__main__":
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
    parser.add_argument('-class_suffix', '-cs', type=str, default="Base",
                        help='suffix to append to class name (default: Base)')
    parser.add_argument('-template_hpp', '-thpp', type=str, help='path to HPP template file')
    parser.add_argument('-template_cpp', '-tcpp', type=str, help='path to CPP template file')
    parser.add_argument('-dest_hpp', '-dhpp', type=str,
                        help='path to file in which to store generated HPP content (default: ClassSuffixBase.hpp)')
    parser.add_argument('-dest_cpp', '-dcpp', type=str,
                        help='path to file in which to store generated CPP content (default: ClassSuffixBase.cpp)')
    parser.add_argument('-dest_dir', '-d', type=str,
                        help='path to folder where to store generated files (ignored if -dest_hpp and -dest_cpp are provided)')

    parser.add_argument('-out', '-o', type=str, help='path for storing generated Plantuml file (only for -plantuml)')

    args = parser.parse_args()

    if args.code:
        if (args.scxml is None) or (args.class_name is None) or (args.template_hpp is None) or (
                args.template_cpp is None):
            print("ERROR: scxml, class_name, template_hpp or template_cpp was not provided")
            exit(1)
        if ((args.dest_hpp is None) and (args.dest_cpp is not None)) or (
                (args.dest_hpp is not None) and (args.dest_cpp is None)):
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