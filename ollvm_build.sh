# 清空目录
rm -rf ../llvm-project13-build && mkdir ../llvm-project13-build && cd ../llvm-project13-build
# 生成Ninja构建
cmake -G "Ninja" -DLLDB_CODESIGN_IDENTITY='' -DCMAKE_BUILD_TYPE=MinSizeRel -DLLVM_APPEND_VC_REV=on -DLLDB_USE_SYSTEM_DEBUGSERVER=YES -DLLVM_CREATE_XCODE_TOOLCHAIN=on -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_INSTALL_PREFIX=~/Library/Developer/ ../llvm-project13/llvm
# 构建
ninja -j8
# 清空xcode工具链
sudo rm -rf ~/Library/Developer/Toolchains/LLVM_hikari13.0.1.xctoolchain
# 重新生成工具链
sudo ninja install-xcode-toolchain
# 拷贝工具链需要的库
sudo rsync -a --ignore-existing /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/ ~/Library/Developer/Toolchains/LLVM_hikari13.0.1.xctoolchain/
sudo rm ~/Library/Developer/Toolchains/LLVM_hikari13.0.1.xctoolchain/ToolchainInfo.plist
# 处理特定文件夹
sudo rsync -a --ignore-existing ~/Library/Developer/Toolchains/LLVM_hikari13.0.1.xctoolchain/usr/lib/clang/13.0.0/ ~/Library/Developer/Toolchains/LLVM_hikari13.0.1.xctoolchain/usr/lib/clang/13.0.1
sudo rm -rf ~/Library/Developer/Toolchains/LLVM_hikari13.0.1.xctoolchain/usr/lib/clang/13.0.0