#!/bin/sh

python3 ../../scxml2gen/scxml2gen.py -plantuml -s ./includes/xi_include_wrapped.scxml -o ../../doc/diagrams/wiki_examples_xiinclude_wrapped.plantuml
python3 ../../scxml2gen/scxml2gen.py -plantuml -s ./includes/xi_include_direct.scxml -o ../../doc/diagrams/wiki_examples_xiinclude_direct.plantuml
python3 ../../scxml2gen/scxml2gen.py -plantuml -s ./includes/state_src_include.scxml -o ../../doc/diagrams/wiki_examples_state_src_include.plantuml
