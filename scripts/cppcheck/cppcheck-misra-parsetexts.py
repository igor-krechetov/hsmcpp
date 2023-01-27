#!/usr/bin/python3
# Author: https://github.com/ChisholmKyle/SublimeLinter-cppcheck-misra

"""Generate MISRA C 2012 rule texts from pdf.

Arguments:
    filename -- text file from parsed MISRA pdf file

Example:
    python3 cppcheck-misra-parsetexts.py "/path/to/MISRA_C_2012.pdf"

"""

import os
import re
import sys
import json
import tempfile
import subprocess

# rules
_appendixa_regex = re.compile(r'Appendix A Summary of guidelines\n')
_appendixb_regex = re.compile(r'Appendix B Guideline attributes\n')
_rule_regex = re.compile(
    r'(Rule|Dir).(\d+)\.(\d+)\n\n(Advisory|Required|Mandatory)\n\n([^\n]+)\n')
_line_regex = re.compile(r'([^\n]+)\n')


def misra_dict_to_text(misra_dict):
    """Convert dict to string readable by cppcheck's misra.py addon."""
    misra_str = ''
    for num1 in misra_dict:
        for num2 in misra_dict[num1]:
            misra_str += '\n{} {}.{} {}\n'.format(
                misra_dict[num1][num2]['type'], num1, num2, misra_dict[num1][num2]['category'])
            misra_str += '{}\n'.format(misra_dict[num1][num2]['text'])
    return misra_str


def parse_misra_xpdf_output(misra_file):
    """Extract misra rules texts from xPDF output."""
    misra_dict = {}

    with open(misra_file, 'r', encoding="utf-8") as fp:
        fp_text = fp.read()

    # end of appendix A
    appb_end_res = _appendixb_regex.search(fp_text)
    last_index = appb_end_res.regs[0][0]

    appres = _appendixa_regex.search(fp_text)
    if appres:
        start_index = appres.regs[0][1]
        res = _rule_regex.search(fp_text, start_index)
        while res:
            start_index = res.regs[0][1]
            ruletype = res.group(1)
            rulenum1 = res.group(2)
            rulenum2 = res.group(3)
            category = res.group(4)
            ruletext = res.group(5).strip()
            statereadingrule = True
            while statereadingrule:
                lineres = _line_regex.match(fp_text, start_index)
                if lineres:
                    start_index = lineres.regs[0][1]
                    stripped_line = lineres.group(1).strip()
                    ruletext += ' ' + stripped_line
                else:
                    # empty line, stop reading text and save to dict
                    if rulenum1 not in misra_dict:
                        misra_dict[rulenum1] = {}
                    misra_dict[rulenum1][rulenum2] = {
                        'type': ruletype,
                        'category': category,
                        'text': ruletext
                    }
                    statereadingrule = False
            res = _rule_regex.search(fp_text, start_index)
            if res and (last_index < res.regs[0][0]):
                break
    fp.close()

    return misra_dict


def misra_parse_pdf(misra_pdf):
    """Extract misra rules texts from Misra-C-2012 pdf."""

    if not os.path.isfile(misra_pdf):
        print('Fatal error: PDF file is not found: ' + misra_pdf)
        sys.exit(1)
    f = tempfile.NamedTemporaryFile(delete=False)
    f.close()
    subprocess.call([
        'pdftotext',
        '-enc', 'UTF-8',
        '-eol', 'unix',
        misra_pdf,
        f.name
    ])
    misra_dict = parse_misra_xpdf_output(f.name)
    os.remove(f.name)

    return misra_dict


misra_pdf_filename = sys.argv[1]

misra_dict = misra_parse_pdf(misra_pdf_filename)
misra_text = 'Appendix A Summary of guidelines\n\n' + \
             misra_dict_to_text(misra_dict)

misra_json_fout = os.path.splitext(misra_pdf_filename)[0] + '_Rules.json'
misra_text_fout = os.path.splitext(misra_pdf_filename)[0] + '_Rules.txt'

with open(misra_json_fout, 'w', encoding='utf-8') as fp:
    fp.write(json.dumps(misra_dict, indent=4))
print('Done creating "' + misra_json_fout + '"')

with open(misra_text_fout, 'w', encoding='utf-8') as fp:
    fp.write(misra_text)
print('Done creating "' + misra_text_fout + '"')