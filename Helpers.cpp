#include "pch.h"
#include "Helpers.h"

unsigned char* t::load_binary_file(const char* filepath, size_t* buffer_size)
{
    FILE* file = nullptr;
    if (fopen_s(&file, filepath, "rb") != 0)
    {
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    *buffer_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char* buffer = reinterpret_cast<unsigned char*>(malloc(*buffer_size));
    if (!buffer)
    {
        fclose(file);
        return nullptr;
    }

    if (fread(buffer, 1, *buffer_size, file) != *buffer_size)
    {
        fclose(file);
        free(buffer);
        return nullptr;
    }

    fclose(file);
    return buffer;
}