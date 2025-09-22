# xtml

xtml is a C++ utility designed to process special markup files and generate corresponding HTML files. The main features revolve around parsing custom tags, variable substitution, and basic preprocessing to streamline the conversion from template-like source files to ready-to-use HTML.

## Features

- **Variable Parsing and Substitution**: Uses `@var` syntax and replaces variable placeholders (`{{@varname}}`) within templates.
- **Block Processing**: Detects and processes custom `<xtml>...</xtml>` blocks.
- **Content Cleanup**: Removes comments and trims unnecessary whitespace.
- **Expression Evaluation**: Supports string and numeric expressions within variables.
- **File Utilities**: Includes several helper functions for file operations and string manipulation.

## Usage

Build the project using your preferred C++ toolchain. The main executable supports the following commands:

```sh
xtml version
```
Displays the current version of xtml.

```sh
xtml build <input_file>
```
Processes the `<input_file>`, replaces variables, and outputs an HTML file in the same directory.

### Example Workflow

1. Prepare a template file (e.g., `template.xtml`) containing variable definitions and HTML structure.
2. Run:
   ```sh
   xtml build template.xtml
   ```
3. The processed HTML file will be generated in the same folder.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Author

Developed by [Andreas Wagner](https://github.com/Andy16823).
