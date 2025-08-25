#!/usr/bin/env python3
"""
SOL2 to EmmyLua Annotation Generator
Parses C++ files with SOL2 usertype registrations and generates Lua type annotations.
Searches directories recursively for C++ files.
"""

# vibe coded. TODO

import re
import sys
import os
import argparse
from pathlib import Path
from typing import List, Dict, Optional, Set

class Sol2Parser:
    def __init__(self):
        self.current_class = None
        self.fields = []
        self.methods = []
        self.constructors = []
        self.class_comment = ""
        
    def parse_file(self, filename: str) -> Dict[str, str]:
        """Parse a C++ file and return generated Lua annotations."""
        try:
            with open(filename, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
            
            return self.parse_content(content, filename)
        except Exception as e:
            print(f"Error parsing {filename}: {e}")
            return {}
    
    def parse_content(self, content: str, filename: str = "") -> Dict[str, str]:
        """Parse C++ content and return generated Lua annotations."""
        annotations = {}
        
        # Find all usertype registrations
        usertype_pattern = r'lua\.new_usertype<([\w:]+)>\([\s\S]*?\);'
        usertype_matches = re.finditer(usertype_pattern, content, re.MULTILINE)
        
        for match in usertype_matches:
            usertype_block = match.group(0)
            class_name = match.group(1)
            
            # Extract class comment
            class_comment_match = re.search(r'// @class (\w+) (.*?)\n', usertype_block)
            class_comment = class_comment_match.group(2) if class_comment_match else f"{class_name} type for LuaBSGE"
            
            # Reset parser state
            self.current_class = class_name.split('::')[-1]  # Get just the class name
            self.fields = []
            self.methods = []
            self.constructors = []
            self.class_comment = class_comment
            
            # Parse constructors
            self._parse_constructors(usertype_block)
            
            # Parse fields and methods
            self._parse_fields_and_methods(usertype_block)
            
            # Generate annotation
            annotation = self._generate_annotation(filename)
            annotations[self.current_class] = annotation
        
        return annotations
    
    def _parse_constructors(self, usertype_block: str):
        """Parse constructor definitions."""
        constructors_pattern = r'sol::constructors<([\w:<>,\s]*)>\(\)'
        constructors_match = re.search(constructors_pattern, usertype_block)
        
        if constructors_match:
            constructors_str = constructors_match.group(1)
            # Extract individual constructors
            constructor_matches = re.findall(r'([\w:]+)\(([^)]*)\)', constructors_str)
            for constr_name, params in constructor_matches:
                if constr_name:  # Default constructor
                    self.constructors.append(("new", params))
    
    def _parse_fields_and_methods(self, usertype_block: str):
        """Parse field and method definitions with comments."""
        # Pattern to find field/method entries with comments
        entry_pattern = r'// @(field|method) (\w+) (.*?)\n\s*"(\w+)",'
        entries = re.findall(entry_pattern, usertype_block)
        
        for entry_type, _, comment, name in entries:
            if entry_type == 'field':
                self.fields.append((name, comment))
            elif entry_type == 'method':
                self.methods.append((name, comment))
        
        # Also look for methods without specific field comments
        method_pattern = r'"(\w+)", &[\w:]+::(\w+)'
        method_matches = re.findall(method_pattern, usertype_block)
        
        for name, method_name in method_matches:
            # Only add if not already captured by field comments
            if not any(m[0] == name for m in self.methods):
                self.methods.append((name, f"Method {method_name}"))
    
    def _generate_annotation(self, filename: str) -> str:
        """Generate EmmyLua annotation for the current class."""
        lines = []
        
        # Add source file comment
        lines.append(f"-- Source: {os.path.basename(filename)}")
        
        # Class definition
        lines.append(f"---@class {self.current_class} {self.class_comment}")
        
        # Fields
        for field_name, comment in self.fields:
            # Try to extract type from comment
            type_match = re.match(r'^(\w+)\s+(.*)', comment)
            if type_match:
                field_type = type_match.group(1)
                field_comment = type_match.group(2)
                lines.append(f"---@field {field_name} {field_type} {field_comment}")
            else:
                lines.append(f"---@field {field_name} any {comment}")
        
        lines.append(f"{self.current_class} = {{}}")
        lines.append("")
        
        # Constructors
        for constr_name, params in self.constructors:
            lines.append(f"---@return {self.current_class}")
            if params.strip():
                # Parse parameters if any
                param_lines = []
                param_list = [p.strip() for p in params.split(',') if p.strip()]
                for i, param in enumerate(param_list):
                    param_type = "any"
                    if ':' in param:
                        param_type, param_name = param.split(':', 1)
                        param_lines.append(f"---@param {param_name.strip()} {param_type.strip()}")
                    else:
                        param_lines.append(f"---@param param{i} any")
                
                if param_lines:
                    lines.extend(param_lines)
                lines.append(f"function {self.current_class}.{constr_name}({', '.join([f'param{i}' for i in range(len(param_list))])}) end")
            else:
                lines.append(f"function {self.current_class}.{constr_name}() end")
            lines.append("")
        
        # Methods
        for method_name, comment in self.methods:
            # Check if it's a static method (no colon) or instance method (colon)
            if method_name in ['new'] or not any(c.isalpha() for c in method_name):  # Static methods
                lines.append(f"---{comment}")
                return_match = re.search(r'returns? (\w+)', comment, re.IGNORECASE)
                return_type = return_match.group(1).lower() if return_match else "any"
                lines.append(f"---@return {return_type}")
                lines.append(f"function {self.current_class}.{method_name}() end")
            else:  # Instance methods
                lines.append(f"---{comment}")
                return_match = re.search(r'returns? (\w+)', comment, re.IGNORECASE)
                return_type = return_match.group(1).lower() if return_match else "any"
                lines.append(f"---@return {return_type}")
                lines.append(f"function {self.current_class}:{method_name}() end")
            lines.append("")
        
        # Global registration
        lines.append(f"_G.{self.current_class} = {self.current_class}")
        
        return "\n".join(lines)
    
    def find_cpp_files(self, directories: List[str], extensions: List[str] = None) -> List[str]:
        """Find all C++ files in the given directories recursively."""
        if extensions is None:
            extensions = ['.cpp', '.cc', '.cxx', '.h', '.hpp', '.hh', '.hxx']
        
        cpp_files = []
        for directory in directories:
            if os.path.isfile(directory):
                cpp_files.append(directory)
                continue
                
            for root, _, files in os.walk(directory):
                for file in files:
                    if any(file.endswith(ext) for ext in extensions):
                        cpp_files.append(os.path.join(root, file))
        
        return sorted(set(cpp_files))
    
    def generate_annotations_file(self, input_paths: List[str], output_file: str, 
                                 extensions: List[str] = None, exclude_dirs: List[str] = None):
        """Generate a complete annotations file from multiple C++ files/directories."""
        if exclude_dirs is None:
            exclude_dirs = ['build', 'bin', 'obj', '.git', 'node_modules', 'vendor']
        
        # Find all C++ files
        cpp_files = []
        for path in input_paths:
            if os.path.isfile(path):
                cpp_files.append(path)
            else:
                # Filter out excluded directories
                filtered_files = []
                for file in self.find_cpp_files([path], extensions):
                    if not any(exclude_dir in file for exclude_dir in exclude_dirs):
                        filtered_files.append(file)
                cpp_files.extend(filtered_files)
        
        print(f"Found {len(cpp_files)} C++ files to process")
        
        all_annotations = []
        processed_classes = set()
        
        for cpp_file in cpp_files:
            print(f"Processing: {cpp_file}")
            annotations = self.parse_file(cpp_file)
            for class_name, annotation in annotations.items():
                if class_name not in processed_classes:
                    all_annotations.append(annotation)
                    all_annotations.append("")  # Add spacing between classes
                    processed_classes.add(class_name)
        
        if not all_annotations:
            print("No SOL2 usertype registrations found!")
            return
        
        # Create output directory if it doesn't exist
        os.makedirs(os.path.dirname(os.path.abspath(output_file)), exist_ok=True)
        
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("-- Auto-generated EmmyLua annotations from SOL2 registrations\n")
            f.write("-- Generated by sol2_to_emmylua.py\n")
            f.write("-- Source files processed:\n")
            for cpp_file in cpp_files:
                f.write(f"--   {cpp_file}\n")
            f.write("\n")
            f.write("\n".join(all_annotations))
        
        print(f"Generated {output_file} with {len(processed_classes)} class annotations")

def main():
    parser = argparse.ArgumentParser(description='Generate EmmyLua annotations from SOL2 C++ registrations')
    parser.add_argument('output', help='Output Lua annotation file')
    parser.add_argument('inputs', nargs='+', help='Input C++ files or directories to search')
    parser.add_argument('--extensions', nargs='+', default=['.cpp', '.h', '.hpp'],
                       help='File extensions to search for (default: .cpp .h .hpp)')
    parser.add_argument('--exclude', nargs='+', default=['build', 'bin', 'obj', '.git'],
                       help='Directories to exclude from search')
    
    args = parser.parse_args()
    
    parser = Sol2Parser()
    parser.generate_annotations_file(
        args.inputs, 
        args.output, 
        args.extensions, 
        args.exclude
    )

if __name__ == "__main__":
    main()