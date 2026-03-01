# Skills Development Guide

Skills are reusable task templates that extend MiniCLI's capabilities with specialized workflows.

## What are Skills?

Skills are YAML-based templates that:
- Provide structured prompts for specific tasks
- Define parameters for customization
- Encapsulate domain expertise
- Can be shared and reused across projects

## Skill Structure

A skill is defined in a `skill.yaml` file:

```yaml
name: my-skill                    # Unique skill identifier
description: What this skill does # Clear description of when to use it
version: 1.0.0                    # Semantic version
author: your-name                 # Author name

parameters:                       # Input parameters
  - name: param1
    type: string                  # string, number, or boolean
    description: Parameter desc
    required: true                # Whether required
    default: value                # Default value (optional)

prompt: |                         # Prompt template
  Your detailed prompt here
  Use {{param1}} to inject parameters
```

## Creating a New Skill

### 1. Create Skill Directory

```bash
mkdir -p ~/.minicli/skills/my-skill
cd ~/.minicli/skills/my-skill
```

### 2. Write skill.yaml

```yaml
name: code-review
description: Perform comprehensive code review on specified files
version: 1.0.0
author: minicli

parameters:
  - name: files
    type: string
    description: Comma-separated list of files to review
    required: true
    
  - name: focus
    type: string
    description: Areas to focus on (security, performance, style, all)
    required: false
    default: all

prompt: |
  Please review the following files: {{files}}
  Focus areas: {{focus}}
  
  Provide:
  1. Overall assessment
  2. Specific issues found
  3. Suggestions for improvement
```

### 3. Test Your Skill

```bash
# Use the skill tool
/skill code-review files="src/index.ts,src/agent.ts" focus="security"
```

## Parameter Types

### String
```yaml
- name: message
  type: string
  description: A text message
  required: true
```

### Number
```yaml
- name: count
  type: number
  description: Number of iterations
  required: false
  default: 10
```

### Boolean
```yaml
- name: verbose
  type: boolean
  description: Enable verbose output
  required: false
  default: false
```

## Best Practices

### 1. Clear Descriptions
Write descriptions that help the AI understand when to use the skill:

```yaml
# Good
description: Perform a code review on specified files focusing on security, performance, and code quality

# Bad
description: Review code
```

### 2. Meaningful Parameters
Define parameters that capture essential inputs:

```yaml
parameters:
  - name: target_files
    type: string
    description: Files or directories to analyze
    required: true
    
  - name: analysis_type
    type: string
    description: Type of analysis (quick, deep, comprehensive)
    default: quick
```

### 3. Comprehensive Prompts
Provide detailed, step-by-step instructions:

```yaml
prompt: |
  Analyze the following files: {{target_files}}
  Analysis type: {{analysis_type}}
  
  Steps:
  1. Read all files in the target
  2. Identify potential issues
  3. Generate a detailed report with:
     - Summary of findings
     - Severity ratings
     - Specific recommendations
     - Code examples for fixes
```

### 4. Use Examples
Include examples in your prompt:

```yaml
prompt: |
  Create a React component with these specifications:
  Component name: {{component_name}}
  Features: {{features}}
  
  Example structure:
  ```typescript
  interface {{component_name}}Props {
    // props definition
  }
  
  export function {{component_name}}({ props }: {{component_name}}Props) {
    // component implementation
  }
  ```
```

### 5. Handle Edge Cases
Consider what happens when parameters are missing or invalid:

```yaml
prompt: |
  {{#if files}}
  Analyzing files: {{files}}
  {{else}}
  No files specified. Please provide the target files.
  {{/if}}
```

## Skill Organization

### Directory Structure
```
~/.minicli/skills/
├── code-review/
│   └── skill.yaml
├── frontend-design/
│   └── skill.yaml
├── testing/
│   └── skill.yaml
└── documentation/
    └── skill.yaml
```

### Naming Conventions
- Use lowercase with hyphens: `code-review`, `api-testing`
- Be descriptive: `performance-analysis` not `perf`
- Group related skills: `testing-unit`, `testing-e2e`

## Example Skills

### Code Review Skill
```yaml
name: code-review
description: Perform comprehensive code review
version: 1.0.0
author: minicli

parameters:
  - name: files
    type: string
    description: Files to review
    required: true
    
  - name: focus
    type: string
    description: Review focus (security|performance|style|all)
    default: all

prompt: |
  Review these files: {{files}}
  Focus: {{focus}}
  
  Check for:
  1. Security vulnerabilities
  2. Performance issues
  3. Code style and best practices
  4. Potential bugs
  5. Documentation completeness
```

### API Documentation Skill
```yaml
name: api-docs
description: Generate API documentation from code
version: 1.0.0
author: minicli

parameters:
  - name: source_dir
    type: string
    description: Directory containing API route definitions
    required: true
    
  - name: format
    type: string
    description: Output format (markdown|openapi|html)
    default: markdown

prompt: |
  Generate API documentation from: {{source_dir}}
  Format: {{format}}
  
  Include:
  - Endpoint paths and methods
  - Request/response schemas
  - Authentication requirements
  - Example requests
  - Error codes
```

## Sharing Skills

### Publishing
Share your skills via:
- GitHub repositories
- npm packages
- Company internal registries

### Installing Shared Skills
```bash
# Clone from Git
git clone https://github.com/user/minicli-skills.git ~/.minicli/skills/shared

# Or copy individual skill
cp -r downloaded-skill ~/.minicli/skills/
```

## Advanced Topics

### Conditional Logic
Use handlebars-style conditionals:

```yaml
prompt: |
  {{#if include_tests}}
  Generate unit tests for the implementation.
  {{/if}}
  
  {{#unless minimal}}
  Include detailed comments and documentation.
  {{/unless}}
```

### Loops
Iterate over array parameters:

```yaml
prompt: |
  Analyze these endpoints:
  {{#each endpoints}}
  - {{this}}
  {{/each}}
```

### Partial Templates
Include reusable template parts:

```yaml
# In ~/.minicli/skills/shared/_header.yaml
partials:
  header: |
    # Code Analysis Report
    Generated: {{timestamp}}
    Target: {{target}}

# In your skill
prompt: |
  {{> header}}
  
  ## Findings
  ...
```

## Troubleshooting

### Skill Not Found
```bash
# Check skill location
ls ~/.minicli/skills/my-skill/

# Verify skill.yaml exists
cat ~/.minicli/skills/my-skill/skill.yaml
```

### Parameters Not Working
- Ensure parameter names in prompt match exactly: `{{param_name}}`
- Check required parameters are provided
- Verify parameter types are correct

### Prompt Not Effective
- Make prompts more specific and detailed
- Include examples
- Add step-by-step instructions
- Test and iterate

## Resources

- [Example Skills Repository](https://github.com/minicli/skills)
- [Skill Best Practices](https://docs.minicli.dev/skills)
- [Community Skills](https://github.com/minicli/community-skills)
