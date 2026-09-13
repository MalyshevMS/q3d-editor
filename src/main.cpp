#include <print>
#include <q3d/res/vfs.hpp>

int main(int argc, char const *argv[]) {
    q3d::vfs::VFS vfs;

    vfs.openArchive("res/pak001.q3d.tar.zst");

    std::println("Files:");
    for (auto i : vfs.listFiles()) {
        std::println("{}", i);
    }

    std::println();

    auto data = vfs.readFile("hello/hi.txt");

    std::println("{}", data->asStringView());

    return 0;
}
