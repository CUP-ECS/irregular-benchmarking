import re

def cleanup_file(file_path, cleaned_path):
    # Read the file
    with open(file_path, 'r') as file:
        lines = file.readlines()

    # Clean up lines
    cleaned_lines = []
    pattern = r'^PARAM: .+? - \d+$'
    for line in lines:
        if re.match(pattern, line):
            cleaned_lines.append(line)


    # Write cleaned lines back to the file
    with open(cleaned_path, 'w') as file:
        file.writelines(cleaned_lines)

# Provide the path to your file
file_path = '../data/CLAMR-BR/16/results.txt'
cleaned_path = '../data/CLAMR-BR/16/results-cleaned.txt'

# Clean up the file
cleanup_file(file_path, cleaned_path)
