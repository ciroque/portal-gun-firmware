# Contribution workflow

Use focused branches and pull requests for changes to `main`. Name branches for
their purpose, for example `feat/controller-core`, `fix/trigger-debounce`, or
`docs/build-instructions`. Keep each PR centered on a cohesive change; split
independent work into separate PRs.

Use [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/):

```text
type(scope): concise description
```

Use an appropriate type such as `feat`, `fix`, `refactor`, `test`, `docs`, `build`,
`ci`, or `chore`. Scope is optional but useful, for example `controller`, `audio`,
or `config`. Mark breaking changes with `!` or a `BREAKING CHANGE:` footer.

Group commits logically to make code review straightforward. Each commit should
have one clear purpose and include the related implementation, tests, and
documentation where appropriate. Avoid mixing unrelated cleanup with behavior
changes or splitting a cohesive change into arbitrary file-by-file commits.
Keep intermediate commits buildable and relevant checks passing where practical.

PR descriptions should explain the problem, resulting behavior, validation, and
any material limitations. Reference relevant ADRs when they explain the change.
Run checks appropriate to the change before requesting review; distinguish host
tests and successful builds from hardware behavior that has actually been tested.
