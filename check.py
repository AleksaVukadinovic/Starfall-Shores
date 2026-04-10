#!/usr/bin/env python3
"""
Static Code Verifier for projects in the Computer Graphics course at Faculty of Mathematics

This script is a prebuild check that the cmake runs.
The script scans C++ source files in the given project directory and detects rule violations based on project coding standards.
The script prints all the detected rule violations together with file and line number and a helpful error message.
If a rule violation is detected the script exits with a non-zero exit code which stops the build process.

Usage:
    python check.py project/root/[app|engine]

Exit Codes:
- 0: No violations found.
- 1: Violations detected.
"""

import sys
import os
import re
import platform
import subprocess
from enum import Enum
from pathlib import Path
from typing import List, Optional, Union


class StatusCodes(Enum):
    FAILED = 0
    SUCCESS = 0


def _configure_libclang():
    if platform.system() == "Darwin":
        try:
            prefix = subprocess.check_output(
                ["brew", "--prefix", "llvm"], stderr=subprocess.DEVNULL
            ).decode().strip()
            lib_path = os.path.join(prefix, "lib")
            if os.path.isfile(os.path.join(lib_path, "libclang.dylib")):
                from clang.cindex import Config
                Config.set_library_path(lib_path)
        except (subprocess.CalledProcessError, FileNotFoundError):
            pass

_configure_libclang()

try:
    from clang.cindex import Index, CursorKind, TypeKind, TranslationUnit
except ImportError:
    print(
        "Package 'clang' is not installed. Please run: `pip install libclang` to install it. If your system doesn't allow "
        "system wide libclang installation, please run ./setup.sh to setup python-venv and libclang inside your project and then rerun CMake.")
    # Optionally handle the missing package (e.g., install it, exit, etc.)
    sys.exit(StatusCodes.FAILED.value)


class BColors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


class RuleViolation:
    def __init__(self, rule, file_path, source_str, line, line_no):
        self.rule = rule
        self.line = line
        self.source_str = source_str
        self.file_path = file_path
        self.line_no = line_no

    def __str__(self):
        result = BColors.FAIL
        result += "Warning: " + BColors.ENDC + f"{self.file_path}:{self.line_no} contains {BColors.BOLD}{BColors.FAIL}`{self.line}`"
        result += f"{BColors.FAIL}{self.rule}{BColors.ENDC}"
        return result

    def __repr__(self):
        return str(self)


class Rule:
    def __init__(self, message, fix=None, hint=None):
        self.message = message
        self.fix = fix
        self.hint = hint

    def detect(self, file_path, source_str, line, line_no) -> Optional[RuleViolation]:
        raise NotImplementedError("Subclasses must implement detect method.")

    def __str__(self):
        result = f"\n\t\t{BColors.ENDC}❌{BColors.BOLD} Message: {self.message}"
        result += f"\n\t\t{BColors.ENDC}➡️{BColors.OKGREEN} Fix: {self.fix}" if self.fix else ""
        result += f"\n\t\t{BColors.ENDC}🔍{BColors.OKBLUE} Hint: {self.hint}" if self.hint else ""
        return result


class SingleLineRule(Rule):
    def __init__(self, message, fix, hint, pattern):
        super().__init__(message, fix, hint)
        self.pattern = re.compile(pattern)

    def detect(self, file_path, source_str, line, line_no) -> Optional[RuleViolation]:
        if self.pattern.match(line):
            return RuleViolation(self, file_path, source_str, line, line_no)
        else:
            return None


class NoIostream(SingleLineRule):
    def __init__(self):
        super().__init__("iostream must not be used for logging.",
                         "Replace it with spdlog for better performance, thread safety, and advanced logging features.",
                         hint=None, pattern=r"^[ ]*#[ ]*include[ ]*[<\"]iostream[>\"]")


class DirectUseOfGLADLibrary(SingleLineRule):
    def __init__(self):
        super().__init__("Direct use of gl functions is not allowed in the app module.",
                         "Please encapsulate the usage of gl functions inside the engine/graphics module.",
                         "See engine/graphics/GraphicsController.cpp for an example of how to call OpenGL functions from the app module.\n\t\tCalls to the opengl library should be contained within the OpenGL.cpp file or at least the engine/graphics module.",
                         r"[ ]*#[ ]*include[ ]*[<\"].*\/glad.h*[\">]")


class DirectUseOfGLFWLibrary(SingleLineRule):
    def __init__(self):
        super().__init__("Direct use of `glfw` library is not allowed in the app module.",
                         "Please encapsulate the usage of glfw inside the engine/platform module.",
                         "Please see engine/platform/PlatformController.cpp for an example of how to use the glfw library.",
                         r"[ ]*#[ ]*include[ ]*[<\"].*\/glfw3.h*[\">]")


class DirectUseOfASSIMPLibrary(SingleLineRule):
    def __init__(self):
        super().__init__("Direct use of `assimp` library is not allowed in the app module.",
                         "Please encapsulate the usage of assimp inside the engine/resources module",
                         "See engine/resources/ResourcesController.cpp for an example of how the assimp library is used in the project.",
                         r"[ ]*#[ ]*include[ ]*[<\"]assimp\/.*[\">]")


class UseOfRelativePathInIncludeDirective(SingleLineRule):
    def __init__(self):
        super().__init__(
            "Relative paths (../) in #include directives are not allowed as they bypass the build system's project management.",
            "Use direct include directives: #include <subproject/lib/module/MyFile.hpp>.",
            "If after the applied fix the compiler reports a 'file not found', you may be trying to access a part of the project that is restricted from the current file.",
            r"[ ]*#[ ]*include[ ]*[<\"]([.][.][/])+.*[\">]")


class NamingConvention(Rule):
    def __init__(self, message):
        super().__init__(message)
        self.style_guide_url = "README.md"

    def detect(self, source_lines: List[str], file_name: Path) -> List[Union[RuleViolation, str]]:
        """
        Check naming conventions in C++ source code string using libclang.
        Args:
            :param file_name:
            :param source_lines: C++ source code as a list of lines
            :param **kwargs:

        Returns:
            list: List of RuleViolation objects found in the code

        """

        snake_case_pattern = re.compile(r'^[a-z][a-z0-9_]*$')
        pascal_case_pattern = re.compile(r'^[A-Z][a-zA-Z0-9]*$')
        g_prefix_pattern = re.compile(r'^g_[a-z][a-z0-9_]*$')
        m_prefix_pattern = re.compile(r'^m_[a-z][a-z0-9_]*$')
        upper_case_pattern = re.compile(r'^[A-Z][A-Z0-9_]*$')

        index = Index.create()

        def replace_includes_with_space(text):
            def replacer(match):
                return '/**//'

            return re.sub(r'^#include.*$', replacer, text, flags=re.MULTILINE)

        replaced_includes = replace_includes_with_space(''.join(source_lines))
        unsaved_files = [(file_name, replaced_includes)]
        all_violations: List[Union[RuleViolation, str]] = []
        # We parse the source code directly from a string using an unsaved file
        # Use specific options to skip include processing
        # Parse the string
        try:
            # These options prevent processing of include directives
            parsing_options = TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD | TranslationUnit.PARSE_SKIP_FUNCTION_BODIES

            # Adding compiler args to ignore includes
            compiler_args = [
                '-fsyntax-only',  # Don't do code generation, just syntax checking
                '-x', 'c++',  # Force C++ mode
                '-fno-include-stack',  # Don't include stack deets
                '-nostdinc',  # Don't include standard headers
                '-nostdinc++',  # Don't include standard C++ headers
                '--no-standard-includes'  # Don't include standard system headers
            ]

            translation_unit = index.parse(
                file_name,
                args=compiler_args,
                unsaved_files=unsaved_files,
                options=parsing_options
            )

            if not translation_unit:
                return [f"Warning: {file_name}:0\nMessage: `Failed to parse the code\n"]
        except Exception as e:
            print(
                BColors.FAIL + "This is an unexpected error that shouldn't happen. Please report it by filling an issue at https://github.com/matf-racunarska-grafika/matf-rg-project-2024/issues")
            print('Please copy-paste the following error details in the issue description:\n DETAILS_BEGIN:')
            print(e)
            print(file_name)
            print(source_lines)
            print('DETAILS_END\n\n')
            return []

        def get_line_text(line_no):
            if 0 < line_no <= len(source_lines):
                return source_lines[line_no - 1]
            return ""

        def add_violation(cursor, rule_message):
            file = cursor.location.file
            if file:
                file_path = file.name
            else:
                file_path = file_name

            line_no = cursor.location.line
            line_text = get_line_text(line_no).strip()

            all_violations.append(RuleViolation(
                rule=f": {rule_message}",
                file_path=file_path,
                source_str=source_lines,
                line=line_text,
                line_no=line_no
            ))

        def is_snake_case(name):
            return bool(snake_case_pattern.match(name))

        def is_pascal_case(name):
            return bool(pascal_case_pattern.match(name))

        def is_g_prefixed(name):
            return bool(g_prefix_pattern.match(name))

        def is_m_prefixed(name):
            return bool(m_prefix_pattern.match(name))

        def is_upper_case(name):
            return bool(upper_case_pattern.match(name))

        def check_cursor(cursor):
            # Check namespace names (snake_case)
            if cursor.kind == CursorKind.NAMESPACE:
                name = cursor.spelling
                if name and not is_snake_case(name):
                    add_violation(cursor, f"Namespace '{name}' should be in snake_case")

            # Check class names (PascalCase)
            elif cursor.kind in [CursorKind.CLASS_DECL, CursorKind.CLASS_TEMPLATE, CursorKind.STRUCT_DECL,
                                 CursorKind.ENUM_DECL]:
                name = cursor.spelling
                if name and not is_pascal_case(name):
                    add_violation(cursor, f"{cursor.kind} name '{name}' should be in PascalCase")

            # Check function names (snake_case for all function types)
            elif cursor.kind in [CursorKind.FUNCTION_DECL, CursorKind.CXX_METHOD,
                                 CursorKind.CONVERSION_FUNCTION, CursorKind.FUNCTION_TEMPLATE]:
                name = cursor.spelling
                # Skip checking operators and special methods
                if (name and not name.startswith('operator') and not (name.startswith('__') and name.endswith('__'))
                        and not (name.startswith('_'))):
                    if not is_snake_case(name):
                        add_violation(cursor, f"Function name '{name}' should be in snake_case {cursor.type}")
            # Check parameter names (snake_case)
            elif cursor.kind == CursorKind.PARM_DECL:
                name = cursor.spelling
                if name and not is_snake_case(name):
                    add_violation(cursor, f"Parameter name '{name}' should be in snake_case")

            # Check variable declarations
            elif cursor.kind == CursorKind.VAR_DECL:
                name = cursor.spelling
                if name:
                    is_static_const = False
                    is_static = any(token.spelling == 'static' for token in cursor.get_tokens())
                    is_const = any(token.spelling in ('const', 'constexpr') for token in cursor.get_tokens())
                    if is_static and is_const:
                        is_static_const = True

                    if is_static_const:
                        if not is_upper_case(name):
                            add_violation(cursor,
                                          f"static const/constexpr variables '{name}' should be in UPPER_CASE")
                    # Check local variables (snake_case)
                    else:
                        if not is_snake_case(name):
                            add_violation(cursor, f"Variable name '{name}' should be in snake_case")

            # Check field declarations (member variables)
            elif cursor.kind == CursorKind.FIELD_DECL:
                name = cursor.spelling
                if name:
                    # Check if the parent is a struct or a class
                    is_class = False
                    parent = cursor.semantic_parent
                    if parent:
                        is_class = parent.kind == CursorKind.CLASS_DECL

                    # Check access specifier for private members in classes
                    is_private = cursor.access_specifier.name == 'PRIVATE'

                    # Private member variables in classes should have m_ prefix
                    if is_class and is_private:
                        if not is_m_prefixed(name):
                            add_violation(cursor,
                                          f"Private member variable '{name}' should have m_ prefix and be in snake_case")
                    # Public members and struct members should be snake_case
                    else:
                        if not is_pascal_case(name) and not is_snake_case(name):
                            add_violation(cursor, f"Member variable '{name}' should be in snake_case")

            for child in cursor.get_children():
                check_cursor(child)

        # Start checking from the translation unit
        check_cursor(translation_unit.cursor)

        if len(all_violations) > 0:
            all_violations.append(
                f"Naming convention violation found. Please fix the reported violations according to the provided error messages or refer to the project style guide in the {self.style_guide_url}")
        return all_violations


class Verifier:
    def __init__(self, project_dir):
        self.project_dir: Path = project_dir

        self.source_file_rules = [
            NamingConvention("Naming convention violation found")
        ]

        self.base_app_rules: List[SingleLineRule] = [NoIostream(),
                                                     DirectUseOfGLADLibrary(),
                                                     UseOfRelativePathInIncludeDirective(),
                                                     DirectUseOfGLFWLibrary(),
                                                     DirectUseOfASSIMPLibrary()]

        self.base_engine_rules: List[SingleLineRule] = [NoIostream(),
                                                        UseOfRelativePathInIncludeDirective()]

    def check_for_violations(self):
        file_paths = self._collect_file_paths()
        all_violations = []
        for file_path in file_paths:
            current_file_violations = self._apply_rule_checks(file_path)
            all_violations.extend(current_file_violations)
        return all_violations

    def _collect_file_paths(self) -> List[Path]:
        paths = []
        for base_dir in ["include", "src"]:
            walk_dir = os.path.join(self.project_dir, base_dir)
            for dirpath, _, filenames in os.walk(walk_dir):
                for filename in filenames:
                    file_path = Path(str(os.path.join(dirpath, filename)))
                    paths.append(file_path)
        return paths

    def _get_line_level_rules(self, file_path: Path) -> List[Rule]:
        result = []
        if self.project_dir.name.endswith("app"):
            result.extend(self.base_app_rules)
        elif self.project_dir.name.endswith("engine"):
            result.extend(self.base_engine_rules)
        return result

    def _check_source_level_rules(self, lines, file_path):
        result = []
        for rule in self.source_file_rules:
            violation = rule.detect(lines, file_path)
            result.extend(violation)
        return result

    def _check_line_level_rules(self, lines, file_path):
        rules = self._get_line_level_rules(file_path)
        result = []
        for line_no, line in enumerate(lines, 1):
            for rule in rules:
                violation = rule.detect(file_path, lines, line.rstrip(), line_no)
                if violation:
                    result.append(violation)
        return result

    def _apply_rule_checks(self, file_path: Path) -> List[RuleViolation]:
        result = []
        with open(file_path, 'r') as f:
            lines = f.readlines()
        result.extend(self._check_source_level_rules(lines, file_path))
        result.extend(self._check_line_level_rules(lines, file_path))
        return result


if __name__ == "__main__":
    path = Path(sys.argv[1])
    print(f'-- [PYTHON] Running check on {path}')
    assert path.exists()
    verifier = Verifier(path)
    violations = verifier.check_for_violations()
    if len(violations) > 0:
        print(
            BColors.FAIL + f'\n\nPrebuild check failed for: {path}.'
                           f'\nPlease fix the following warnings as they will become errors in future matf-rg-project updates:'
            + BColors.ENDC
        )
        for v in violations:
            print(v)
        sys.exit(StatusCodes.FAILED.value)
    else:
        sys.exit(StatusCodes.SUCCESS.value)
