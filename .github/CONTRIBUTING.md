# Contributing to CustFunc

Thank you for taking the time to contribute!

The following is a set of guidelines for contributing to CustFunc on GitHub. These are mostly guidelines, not rules. Use your best judgment, and feel free to propose changes to this document in a pull request.

#### Table Of Contents

[I don't want to read this whole thing, I just have a question!!!](#i-dont-want-to-read-this-whole-thing-i-just-have-a-question)

[How Can I Contribute?](#how-can-i-contribute)
  * [Reporting Bugs](#reporting-bugs)
  * [Suggesting Enhancements](#suggesting-enhancements)
  * [Your First Code Contribution](#your-first-code-contribution)
  * [Pull Requests](#pull-requests-prs)

[Styleguides](#styleguides)
  * [Git Commit Messages](#git-commit-messages)
  * [C++ Code]

## I don't want to read this whole thing I just have a usage question!!!

> **Note:** Please don't file an issue to ask a usage question. You'll get faster results by using the Discussion Board.

Try the following

* [Open a discussion on the Discussions Tab](https://github.com/henningjp/CustFunc/discussions)
* Review the Instructions at the bottom of the [README](https://github.com/henningjp/CustFunc), displayed on the CustFunc home page.

## How Can I Contribute?

### Reporting Bugs

This section guides you through submitting a bug report for CustFunc. Following these guidelines helps maintainers and the community understand your report, reproduce the behavior, and find related reports.

Before creating bug reports, please check the current [Issues List](https://github.com/henningjp/CustFunc/issues) (even **_closed_** issues) as you might find out that you don't need to create one. When you are creating a bug report from the Issues page, please include as many details as possible. Fill out the required template, the information it asks for helps us to resolve issues faster.

> **Note:** If you find a **Closed** issue that seems like it is the same thing that you're experiencing, and you are using the latest CustFUnc version that includes that issue resolution, open a new issue and include a link to the original issue in the body of your new one.

#### Before Submitting A Bug Report

* **Check the README and the Wiki for a list of common questions and problems.
* **Perform a [cursory search](https://github.com/henningjp/CustFunc/issues)** to see if the problem has already been reported. If it has **and the issue is still open**, add a comment to the existing issue instead of opening a new one.

#### How Do I Submit A (Good) Issue Report?

Issues are tracked as [GitHub issues](https://guides.github.com/features/issues/). After you've determined the need to report an issue, create an issue on the CustFunc repository and provide the following information by filling in the Bug reporting template.

Explain the problem and include additional details to help maintainers reproduce the problem:

* **Use a clear and descriptive title** for the issue to identify the problem.
* **Describe the exact steps which reproduce the problem** with as many details as possible. For example, start by explaining which XML templates you are using, e.g. one of the example XML files or your own, or which button clicks you are using. When listing steps, **don't just say what you did, but explain how you did it**. 
* **Provide specific examples to demonstrate the steps**. Include any custom XML files you are using.
* **Describe the behavior you observed after following the steps** and point out what exactly is the problem with that behavior.
* **Explain which behavior you expected to see instead and why.**
* **Include screenshots and code segments** which show you following the described steps and clearly demonstrate the problem. 
* **If you're reporting that CustFunc crashed or didn't load**, include any error message that were observed.

Provide more context by answering these questions:

* **Did the problem start happening recently** (e.g. after updating to a new version of CustFunc) or was this always a problem?
* If the problem started happening recently, **can you reproduce the problem in an older version of CustFunc, Mathcad Prime or on a different OS?** What's the most recent version in which the problem doesn't happen? 
* **Can you reliably reproduce the issue?** If not, provide details about how often the problem happens and under which conditions it normally happens.
* If the problem is related to working with a specific XML script, **does the problem happen for all XML files or only some?**

Include details about your configuration and environment:

* **Which version of CustFunc are you using?** You can get the exact version by pressing the info button.  
* **What version of Windows are you using**?  
* **Which version of Mathcad Prime are you using (e.g. 10.0.0.1, etc.)  

### Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for CustFunc, including completely new features and minor improvements to existing functionality. Following these guidelines helps maintainers and the community understand your suggestion and find related suggestions.

#### How Do I Submit A (Good) Enhancement Suggestion?

Enhancement suggestions submitted as an **Idea** in a [CustFunc Discussion](https://github.com/henningjp/CustFunc/discussions) post and provide the following information:

* **Use a clear and descriptive title** for the issue to identify the suggestion.
* **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
* **Provide specific examples to demonstrate the steps**. Include copy/pasteable snippets which you use in those examples, as [Markdown code blocks](https://help.github.com/articles/markdown-basics/#multiple-lines).
* **Describe the current behavior** and **explain which behavior you expected to see instead** and why.
* **Include screenshots and animated GIFs** which help you demonstrate the steps or point out the part of CustFunc which the suggestion is related to. 
* **Explain why this enhancement would be useful** to most CustFunc users. 
* **Specify which version of CustFunc you're using.** 
* **Specify the version of Mathcad Prime you're using.**


#### Local development

CustFunc can be developed locally on your machine.  Once code changes are completed and tested, make a Pull Request (PR) to the CustFunc repository.  Please see the [Wiki](https://github.com/henningjp/CustFunc/wiki) on contributing to CustFunc.  

### Pull Requests (PRs)

* Fill in the required template
* Do not include issue numbers in the PR title
* Include screenshots in your pull request whenever possible.
* Follow the [C++ Coding Guidelines] for CustFunc.
* Document new code based on the Documentation Styleguide
* Avoid platform-dependent code 

## Styleguides

### Git Commit Messages

* Use the present tense ("Add feature" not "Added feature")
* Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
* Limit the first line to 72 characters or less
* Reference issues and pull requests liberally after the first line
* When only changing documentation, XML examples, or workflows (i.e. no actual code), include `[ci skip]` in the commit title

