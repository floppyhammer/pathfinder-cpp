import os
import sys
import struct

target_extension = sys.argv[1]

for current_dir, dir_list, file_list in os.walk(r"./"):
    for file_name in file_list:
        if not file_name.endswith("." + target_extension):
            continue

        base_name = os.path.splitext(file_name)[0]
        base_name = base_name.replace(".", "_")
        base_name = base_name.replace("-", "_")
        base_name += "_" + target_extension

        header_file_name = base_name + ".h"

        dst_stream = open(current_dir + "/" + header_file_name, 'w')
        dst_stream.write("#ifndef PATHFINDER_RESOURCE_" + base_name.upper() + "_H\n")
        dst_stream.write("#define PATHFINDER_RESOURCE_" + base_name.upper() + "_H\n\n")

        dst_stream.write("namespace Pathfinder {\n    static uint8_t " + base_name + "[] = {")

        with open(current_dir + "/" + file_name, 'rb') as src_stream:
            file_text = src_stream.read()
            char = struct.unpack('<B', file_text[0:1])
            dst_stream.write(str(int(char[0])))
            for i in range(1, len(file_text), 1):
                char = struct.unpack('<B', file_text[i:i + 1])
                dst_stream.write("," + str(int(char[0])))
        dst_stream.write("};\n")

        dst_stream.write("}\n\n#endif //PATHFINDER_RESOURCE_" + base_name.upper() + "_H\n")
        dst_stream.close()

        print("Converted " + file_name + " to " + header_file_name)
