import logging as log
import re

from tree_sitter import Node

from CppTranslator.Patches.HelperMethods import parse_function_capture
from CppTranslator.Patches.Patch import Patch
from CppTranslator.TemplateCollector import TemplateCollector, TemplateRefInstance


class TemplateDefinition(Patch):
    """
    Patch   template<A, B>
            RET_TYPE TemplateFunction(...) {...}

    to      #define DEFINE_TemplateFunction_A_B \
            RET_TYPE CONCAT(TemplateFunction, CONCAT(A, B))(...) {...}
    """

    def __init__(self, priority: int, template_collector: TemplateCollector):
        self.collector: TemplateCollector = template_collector
        super().__init__(priority)

    def get_search_pattern(self) -> str:
        return (
            "(template_declaration"
            "     ((template_parameter_list) @templ_params)"
            "     (function_definition"
            "        ((storage_class_specifier)* @storage_class_id)"
            "        ([(type_identifier)(primitive_type)] @type_id)"
            "        (function_declarator"
            "            ((identifier) @fcn_name)"
            "            ((parameter_list) @fcn_params)"
            "        )"
            "        ((compound_statement) @compound)"
            "     )"
            ") @template_def"
        )

    def get_main_capture_name(self) -> str:
        return "template_def"

    def get_patch(self, captures: list[tuple[Node, str]], src: bytes, **kwargs) -> bytes:
        t_params, sc, tid, f_name, f_params, f_compound = parse_function_capture(captures, src)
        if f_name in self.collector.templates_with_arg_deduction:
            return sc + tid + b" " + f_name + f_params + f_compound

        definition = b"#define DEFINE_" + f_name + b"(" + b", ".join(t_params) + b")\n"
        definition += (
            sc + b" " + tid + b" " + TemplateCollector.get_macro_c_call(f_name, t_params, f_params) + f_compound
        )
        # Remove // comments
        definition = re.sub(b" *//.*", b"", definition)
        definition = definition.replace(b"\n", b" \\\n") + b"\n"

        template_instance: TemplateRefInstance
        declared_implementations = list()
        if f_name not in self.collector.template_refs:
            self.collector.log_missing_ref_and_exit(f_name)

        for template_instance in self.collector.template_refs[f_name]:
            d = b"DEFINE_" + f_name + b"(" + b", ".join(template_instance.get_args_for_decl()) + b");\n"
            if d in declared_implementations:
                continue
            declared_implementations.append(d)
            definition += d
        return definition
