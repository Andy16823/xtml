# XTML - Extended Template Markup Language

XTML is a lightweight template processing tool written in C++ that allows you to create dynamic HTML files from template files with variables and expressions.

## Features

- 🔧 **Variable Processing**: Define and use variables within your templates
- 🧮 **Expression Evaluation**: Support for string concatenation and basic numeric operations  
- 📝 **Template Syntax**: Clean, readable syntax with `<xtml>` blocks for logic
- 🚀 **Fast Processing**: Efficient C++ implementation for quick template compilation
- 📁 **File Generation**: Automatically generates HTML files from XTML templates

## Installation

### Building from Source

Requirements:
- C++20 compatible compiler (GCC 10+, Clang 10+, or MSVC 2019+)
- Standard C++ library

#### Linux/macOS
```bash
git clone https://github.com/Andy16823/xtml.git
cd xtml/xtml
g++ -std=c++20 -o xtml *.cpp
```

#### Windows (Visual Studio)
```cmd
git clone https://github.com/Andy16823/xtml.git
cd xtml
# Open xtml.sln in Visual Studio and build the solution
```

## Usage

### Basic Commands

```bash
# Show version
./xtml version

# Build an XTML file to HTML
./xtml build <input_file.xtml>
```

### XTML File Syntax

XTML files are HTML files with special `<xtml>` blocks that contain variable definitions and logic.

#### Variable Declaration
Variables are declared using the `@var` syntax within `<xtml>` blocks:

```html
<xtml>
@var title = "My Website";
@var author = "John Doe";
@var year = 2024;
</xtml>
```

#### Variable Usage
Variables are referenced in the HTML using `{{@variable_name}}` syntax:

```html
<!DOCTYPE html>
<html>
<head>
    <title>{{@title}}</title>
    <meta name="author" content="{{@author}}">
</head>
<body>
    <h1>{{@title}}</h1>
    <p>Copyright {{@year}} {{@author}}</p>
</body>
</html>
```

### Complete Example

**Input file: `example.xtml`**
```html
<!DOCTYPE html>
<html>
<head>
    <title>{{@pageTitle}}</title>
    <meta name="description" content="{{@description}}">
</head>
<body>
    <header>
        <h1>{{@siteName}}</h1>
        <nav>{{@navigation}}</nav>
    </header>
    
    <main>
        <h2>{{@pageTitle}}</h2>
        <p>{{@content}}</p>
        <p>Page {{@pageNumber}} of {{@totalPages}}</p>
    </main>
    
    <footer>
        <p>&copy; {{@year}} {{@author}}. All rights reserved.</p>
    </footer>
</body>
</html>

<xtml>
@var siteName = "My Awesome Site";
@var pageTitle = "Welcome Page";
@var description = "Welcome to my awesome website";
@var navigation = "<a href='/'>Home</a> | <a href='/about'>About</a>";
@var content = "This is the main content of the page.";
@var author = "Jane Developer";
@var year = 2024;
@var pageNumber = 1;
@var totalPages = 10;
</xtml>
```

**Build command:**
```bash
./xtml build example.xtml
```

**Generated output: `example.html`**
```html
<!DOCTYPE html>
<html>
<head>
    <title>Welcome Page</title>
    <meta name="description" content="Welcome to my awesome website">
</head>
<body>
    <header>
        <h1>My Awesome Site</h1>
        <nav><a href='/'>Home</a> | <a href='/about'>About</a></nav>
    </header>
    
    <main>
        <h2>Welcome Page</h2>
        <p>This is the main content of the page.</p>
        <p>Page 1 of 10</p>
    </main>
    
    <footer>
        <p>&copy; 2024 Jane Developer. All rights reserved.</p>
    </footer>
</body>
</html>
```

## Advanced Features

### String Expressions
You can concatenate strings using the `+` operator:

```html
<xtml>
@var firstName = "John";
@var lastName = "Doe";
@var fullName = firstName + " " + lastName;
</xtml>

<p>Hello, {{@fullName}}!</p>
```

### Numeric Expressions
Basic arithmetic operations are supported:

```html
<xtml>
@var price = 100;
@var tax = 20;
@var total = price + tax;
</xtml>

<p>Total: ${{@total}}</p>
```

### Comments
Comments can be added using `#` within XTML blocks:

```html
<xtml>
# This is a comment
@var title = "My Page";
# Another comment
@var author = "Developer";
</xtml>
```

## Project Structure

```
xtml/
├── xtml/
│   ├── xtml.cpp      # Main application entry point
│   ├── Core.cpp      # Core template processing logic
│   ├── Core.h        # Core header file
│   ├── Vars.cpp      # Variable parsing and processing
│   ├── Vars.h        # Variable system header
│   ├── Utils.cpp     # Utility functions
│   └── Utils.h       # Utility header file
├── xtml.sln          # Visual Studio solution file
└── LICENSE           # MIT License
```

## Architecture

- **Core**: Handles template block parsing and content processing
- **Vars**: Manages variable parsing, expression evaluation, and substitution
- **Utils**: Provides utility functions for file handling and string operations

## Version

Current version: 0.0.0.1

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Author

Andreas Wagner (@Andy16823)

---

*XTML - Making HTML templating simple and efficient.*