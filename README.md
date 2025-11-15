# XTML

**XTML** is a C++ utility and DSL for generating dynamic HTML content from templates. It allows you to define variables, evaluate expressions, include external files, and embed logic directly into your HTML templates, providing a flexible workflow for generating web pages.

---

## Features

* **Variables and Placeholders**
  Define variables using `var` and reference them with `{{@varName}}` in your templates.

* **Functions and Expressions**
  Create user-defined functions, return values, and evaluate expressions (numeric, string, or HTML).

* **Control Flow**
  Supports `if`, `else if`, `else`, `while`, and `for` loops with `break` and `continue`.

* **HTML Integration**
  Embed HTML directly using `<xtml>` blocks and render dynamic content with expressions.

* **Static Definitions**
  Use `xtmldef` to define reusable static content blocks.

* **Native Functions**
  Access built-in functions such as `std::randStr` and `std::get` for arrays.

* **Arrays and Data Types**
  Work with arrays and mixed data types consistently.

* **Modular Templates**
  Include external XTML files for reusable template logic.

* **Output Handling**
  Distinguishes between captured printed output and returned values for clean template rendering.

---

## Usage

### Build the Project

Compile the project using your preferred C++ toolchain. The main executable supports the following commands:

```sh
xtml version
```

Displays the current XTML version.

```sh
xtml build <input_file>
```

Processes `<input_file>` and outputs a fully rendered HTML file in the same directory.

---

## Example Workflow

### Template (`template.xtml`)

```xtml
<xtml>
    var title = "XTML Example Page";
    var a = 15;
    var b = 7;
    var comparison = expr {
        if (a > b) { "a is greater than b"; } else { "a is not greater than b"; }
    };
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

### Build and Output

```sh
xtml build template.xtml
```

Output:

```html
<html>
<head>
    <title>XTML Example Page</title>
</head>
<body>
    <p>a is greater than b</p>
</body>
</html>
```

---

## Modules / DLL Extensions

Extend XTML functionality through dynamically loaded C++ modules (DLLs). This allows you to add custom functions usable directly in templates.

### Creating a Module

1. Write a C++ shared library (DLL) exposing functions in the XTML API.
2. Export functions using `extern "C"` for proper linking.
3. Place the compiled DLL in the `modules` folder of your project.

### Using a Module

Call module functions in templates like any built-in function:

```xtml
var sum = MyMath::add(10, 5);
```

Modules are loaded automatically when a function call is encountered, or they can be pre-loaded via a configuration.

---

## License

XTML is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Author

Developed by [Andreas Wagner](https://github.com/Andy16823)
