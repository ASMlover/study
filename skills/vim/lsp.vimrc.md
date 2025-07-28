# lsp.vimrc说明

* 这是一个可以安装coc.nvim（lsp）的最简单版本vimrc脚本
* 为你的系统安装nodejs（目前仅在linux上生效）
  - 为本地用户安装nvm
    * $ curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.5/install.sh | bash
  - 设置环境变量（添加到.bashrc文件末尾）
```bash
export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"
[ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"
```
  - 验证安装的nvm生效
    * $ nvm --version
  - 安装最新的LTS版本的nodejs
    * $ nvm install --lts
  - 设置为默认版本
    * $ nvm alias default node
  - 验证安装生效
    * $ node --version
    * $ npm --version
* 安装vim.plug插件
  - $ curl -fLo ~/.vim/autoload/plug.vim --create-dirs https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
* 安装coc.nvim插件
  - 启动vim
  - 运行`:PlugInstall`
  - 安装需要的语言服务器 `:CocInstall coc-pyright coc-marketplace`
* 配置pyright，`:CocConfig`
```json
{
  "pyright.enable": true,
  "inlayHint.enable": false,
  "python.linting.enabled": true,
  "python.linting.pylintEnabled": true,
  "python.analysis.typeCheckingMode": "basic",
  "python.analysis.exclude": [
      "**/.pyc",
      "**/.pyo"
  ],
  "python.analysis.extraPaths": [
      "x/y/z"
  ]
}
```
