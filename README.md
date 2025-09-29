# XTML

**XTML** is a C++ utility for processing custom markup templates and generating HTML files. It allows you to define variables, evaluate expressions, and include modular content, making HTML generation dynamic and flexible.

---

## Features

* **Variable Parsing & Substitution**: Define variables with `@var` and use placeholders `{{@varname}}` in your templates.
* **Conditional Logic**: Use `@if`, `@else if`, and `@else` for dynamic content generation based on variable values.
* **Block Processing**: Detects and evaluates `<xtml>...</xtml>` blocks for preprocessing content.
* **Expression Evaluation**: Supports numeric and string expressions, including concatenation and math operations.
* **Includes & Modularity**: Include external files and pass variables to them for reusable templates.
* **Content Cleanup**: Strips comments and trims unnecessary whitespace.
* **File Utilities**: Provides helper functions for reading, writing, and manipulating files and paths.

---

## Usage

Build the project using your preferred C++ toolchain. The main executable supports the following commands:

```sh
xtml version
```

Displays the current version of XTML.

```sh
xtml build <input_file>
```

Processes `<input_file>` and outputs a fully rendered HTML file in the same directory.

---

## Example Workflow

1. Create a template file `template.xtml`:

```xtml
<xtml>
    @var title = "XTML Example Page";
    @var greeting = "Hello, " + std::toUpper("world") + "!";
</xtml>

<html>
<head>
    <title>{{@title}}</title>
</head>
<body>
    <h1>{{@greeting}}</h1>
</body>
</html>
```

2. Build the HTML:

```sh
xtml build template.xtml
```

3. The output HTML file will be generated in the same folder:

```html
<html>
<head>
    <title>XTML Example Page</title>
</head>
<body>
    <h1>Hello, WORLD!</h1>
</body>
</html>
```

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Author

Developed by [Andreas Wagner](https://github.com/Andy16823)
