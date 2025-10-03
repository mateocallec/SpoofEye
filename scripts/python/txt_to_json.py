#!/usr/bin/env python3
"""
txt_to_json.py

A general-purpose script that converts a text file with one value per line
into a JSON array. 

Author: Your Name
Date: 2025-10-01
"""

import json
import os

def txt_to_json(input_path: str, output_path: str) -> None:
    """
    Reads a text file line by line and writes the lines into a JSON array.
    
    Parameters:
        input_path (str): Path to the input .txt file.
        output_path (str): Path to the output .json file.
    """

    # Check if the input file exists
    if not os.path.isfile(input_path):
        print(f"Error: Input file '{input_path}' does not exist.")
        return

    # Initialize a list to store all values
    values_list = []

    # Open the input file and read line by line
    with open(input_path, 'r', encoding='utf-8') as infile:
        for line_number, line in enumerate(infile, start=1):
            # Strip whitespace and newline characters
            value = line.strip()
            
            # Skip empty lines
            if value:
                values_list.append(value)
            else:
                print(f"Warning: Skipped empty line {line_number}")

    # Write the list to a JSON file
    try:
        with open(output_path, 'w', encoding='utf-8') as outfile:
            json.dump(values_list, outfile, indent=4)
        print(f"Successfully wrote {len(values_list)} entries to '{output_path}'")
    except Exception as e:
        print(f"Error writing JSON file: {e}")

def main():
    """
    Main function that prompts the user for input and output file paths,
    and calls the conversion function.
    """
    print("=== TXT to JSON Converter ===")
    input_path = input("Enter the path to the input .txt file: ").strip()
    output_path = input("Enter the path to the output .json file: ").strip()

    txt_to_json(input_path, output_path)

if __name__ == "__main__":
    main()
