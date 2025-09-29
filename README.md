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
* **Module / DLL Support**: Extend XTML with custom C++ functions loaded dynamically via DLLs.

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
    @var num1 = 15;
    @var num2 = 7;
    @if (num1 > num2) {
        @var comparison = "num1 is greater than num2";
    } @else {
        @var comparison = "num1 is not greater than num2";
    }
</xtml>

<html>
<head>
    <title>{{@title}}</title>
</head>
<body>
    <p>{{@comparison}}</p>
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
    <p>num1 is greater than num2</p>
</body>
</html>
```

---

## Modules / DLL Extensions

XTML supports extending functionality through dynamically loaded C++ modules (DLLs). This allows you to add custom functions, math operations, or other utilities that can be used inside XTML templates.

### Creating a Module

1. Write a C++ shared library (DLL) exposing functions in the expected XTML API.
2. Export functions using `extern "C"` to ensure proper linking.
3. Place the compiled DLL in the `modules` folder of your XTML project.

### Using a Module in XTML

Once the module is loaded, its functions can be used in expressions like any built-in function:

```xtml
@var num1 = 10;
@var num2 = 5;
@var sum = XtmlMath::add(num1, num2);
```

Where `XtmlMath` is the name of your DLL module and `add` is an exported function.

### Loading Modules

Modules are automatically loaded at runtime when XTML encounters a call to a function in a module. You can also pre-load modules by specifying them in a configuration file (if implemented).

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Author

Developed by [Andreas Wagner](https://github.com/Andy16823)
