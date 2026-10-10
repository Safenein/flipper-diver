{
  pkgs,
  config,
  ...
}: let
  # ufbt is not in nixpkgs: pip installs it into a venv kept off PATH, so that
  # the FHS wrapper below is the only `ufbt` the shell sees.
  ufbtVenv = "${config.devenv.state}/ufbt-venv";

  # ufbt downloads a prebuilt ARM toolchain (gcc, python, openocd) linked
  # against glibc. Without nix-ld those binaries cannot start, so ufbt runs
  # inside an FHS environment.
  ufbt-fhs = pkgs.buildFHSEnv {
    name = "ufbt";
    targetPkgs = p: [
      p.zlib
      p.ncurses
      p.libusb1
      p.stdenv.cc.cc.lib
    ];
    runScript = "${ufbtVenv}/bin/ufbt";
  };
in {
  languages.c = {
    enable = true;
    lsp.enable = false;
  };

  languages.python = {
    enable = true;
    lsp.enable = false;
  };

  tasks."flipper-diver:ufbt-venv" = {
    exec = ''
      python -m venv "${ufbtVenv}"
      "${ufbtVenv}/bin/pip" install --quiet -r ${./requirements.txt}
      cp ${./requirements.txt} "${ufbtVenv}/requirements.txt"
    '';
    status = ''
      cmp -s ${./requirements.txt} "${ufbtVenv}/requirements.txt"
    '';
    before = ["devenv:enterShell" "devenv:enterTest"];
  };

  packages = [ufbt-fhs];

  scripts = {
    build.exec = ''ufbt "$@"'';
    launch.exec = ''ufbt launch "$@"'';
    fmt.exec = ''clang-format -i $(git ls-files '*.c' '*.h')'';
  };

  git-hooks.hooks.clang-format = {
    enable = true;
    types_or = ["c"];
  };

  # Project-local devenv MCP server: devenv writes .mcp.json from this.
  claude.code = {
    enable = true;
    mcpServers.devenv = {
      type = "stdio";
      command = "devenv";
      args = ["mcp"];
      env.DEVENV_ROOT = config.devenv.root;
    };
    # The user-level Claude Code config already runs `pre-commit run --files`
    # after each edit; devenv's own copy of that hook would run it twice.
    hooks.git-hooks-run.enable = false;
  };

  enterTest = ''
    ufbt
    clang-format --dry-run --Werror $(git ls-files '*.c' '*.h')
  '';
}
