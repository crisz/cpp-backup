#include <iostream>

// Controlla se una cartella esiste ed è scrivibile
bool check_dest_dir(const std::string& dir);

// Rimuove la prima cartella in un path
std::string remove_first_folder(const std::string& path);