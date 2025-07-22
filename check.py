#!/usr/bin/env python3
"""
Static Code Verifier for projects in the Computer Graphics course at Faculty of Mathematics

This script is a prebuild check that the cmake runs.
The script scans C++ source files in the given project directory and detects rule violations based on project coding standards.
The script prints all the detected rule violations together with file and line number and a helpful error message.
If a rule violation is detected the script exits with a non-zero exit code which stops the build process.

Usage:
    python verifier.py project/root/[app|engine]

Exit Codes:
- 0: No violations found.
- 1: Violations detected.
"""

import sys
import os
import re
from pathlib import Path
from typing import List, Optional


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
                         f"[ ]*#[ ]*include[ ]*[<\"].*\/glad.h*[\">]")


class DirectUseOfGLFWLibrary(SingleLineRule):
    def __init__(self):
        super().__init__("Direct use of `glfw` library is not allowed in the app module.",
                         "Please encapsulate the usage of glfw inside the engine/platform module.",
                         "Please see engine/platform/PlatformController.cpp for an example of how to use the glfw library.",
                         f"[ ]*#[ ]*include[ ]*[<\"].*\/glfw3.h*[\">]")


class DirectUseOfASSIMPLibrary(SingleLineRule):
    def __init__(self):
        super().__init__("Direct use of `assimp` library is not allowed in the app module.",
                         "Please encapsulate the usage of assimp inside the engine/resources module",
                         "See engine/resources/ResourcesController.cpp for an example of how the assimp library is used in the project.",
                         f"[ ]*#[ ]*include[ ]*[<\"]assimp\/.*[\">]")


class UseOfRelativePathInIncludeDirective(SingleLineRule):
    def __init__(self):
        super().__init__(
            "Relative paths (../) in #include directives are not allowed as they bypass the build system's project management.",
            "Use direct include directives: #include <subproject/lib/module/MyFile.hpp>.",
            "If after the applied fix the compiler reports a 'file not found', you may be trying to access a part of the project that is restricted from the current file.",
            f"[ ]*#[ ]*include[ ]*[<\"]([.][.][/])+.*[\">]")


class Verifier:
    def __init__(self, project_dir):
        self.project_dir: Path = project_dir

        self.base_app_rules: List[Rule] = [NoIostream(),
                                           DirectUseOfGLADLibrary(),
                                           UseOfRelativePathInIncludeDirective(),
                                           DirectUseOfGLFWLibrary(),
                                           DirectUseOfASSIMPLibrary()]

        self.base_engine_rules: List[Rule] = [NoIostream(),
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

    def _get_rules(self, file_path: Path) -> List[Rule]:
        result = []
        if self.project_dir.name.endswith("app"):
            result.extend(self.base_app_rules)
        elif self.project_dir.name.endswith("engine"):
            result.extend(self.base_engine_rules)
        return result

    def _apply_rule_checks(self, file_path: Path) -> List[RuleViolation]:
        rules = self._get_rules(file_path)
        result = []
        if len(rules) == 0:
            return result
        with open(file_path, 'r') as f:
            lines = f.readlines()
            for line_no, line in enumerate(lines, 1):
                for rule in rules:
                    violation = rule.detect(file_path, lines, line.rstrip(), line_no)
                    if violation:
                        result.append(violation)
        return result


if __name__ == "__main__":
    path = Path(sys.argv[1])
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
        sys.exit(0)
    else:
        sys.exit(0)
