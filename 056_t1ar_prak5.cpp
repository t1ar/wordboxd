#include <iostream>
#include <sstream>
#include "database.h"

Database db;
bool isAdmin = false;

// Trimming manual untuk membersihkan spasi awal dan akhir (pengganti algorithm trim)
std::string trimString(std::string str) {
    int start = 0;
    while(start < str.length() && (str[start] == ' ' || str[start] == '\t')) start++;
    int end = str.length() - 1;
    while(end >= 0 && (str[end] == ' ' || str[end] == '\t')) end--;
    if (start > end) return "";
    return str.substr(start, end - start + 1);
}

// Split string menjadi custom StringList
StringList splitString(std::string str) {
    StringList result;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token = trimString(token);
        if (!token.empty()) result.add(token);
    }
    return result;
}

void displayMovieDetail(Movie* m) {
    std::cout << "\n---- " << m->nama << " ----\nGenre: ";
    for (int i = 0; i < m->genre_names.count; i++) {
        std::cout << m->genre_names.get(i) << (i < m->genre_names.count - 1 ? ", " : "");
    }
    std::cout << "\nJumlah ep: " << (m->ep == 0 ? "-" : std::to_string(m->ep));
    std::cout << "\nJumlah season: " << (m->season == 0 ? "-" : std::to_string(m->season));
    
    std::cout << "\nFilm yang terkait:\n";
    if (m->related_movies.empty()) std::cout << "- Tidak ada\n";
    else {
        for (int i = 0; i < m->related_movies.count; i++) {
            std::cout << "- " << m->related_movies.get(i)->nama << "\n";
        }
    }
    std::cout << "Studio: " << m->studio << "\n";
    std::cout << "Rating: " << (m->total_rating == 0 ? 0 : m->rating) << "\n";
}

void rateMovie(Movie* m) {
    std::cout << "==== Rate " << m->nama << " ====\nRating: ";
    float rate; std::cin >> rate;
    m->addRating(rate);
    std::cout << "Rating terbaru: " << m->rating << "\n";
    db.saveToFile(); // PENTING: Simpan rating ke database fisik
}

void genreMenu() {
    std::string gList[] = {"Action", "Drama", "Fiction", "Comedy", "Animated"};
    std::cout << "==== Pilih Genre ====\n";
    for (int i = 0; i < 5; i++) {
        std::cout << i+1 << ". " << gList[i] << "\n";
    }
    std::cout << "Pilih: ";
    int pil; std::cin >> pil;

    if (pil > 0 && pil <= 5) {
        GenreNode* gNode = db.getGenre(gList[pil-1]);
        if (gNode->movies.empty()) {
            std::cout << "Belum ada film di genre ini.\n";
            return;
        }

        int idx = 0;
        char action;
        while (true) {
            displayMovieDetail(gNode->movies.get(idx));
            std::cout << "[N]ext, [P]revious, [C]lose, [R]ate : ";
            std::cin >> action;
            if (action == 'N' || action == 'n') {
                if (idx < gNode->movies.count - 1) idx++;
            } else if (action == 'P' || action == 'p') {
                if (idx > 0) idx--;
            } else if (action == 'R' || action == 'r') {
                rateMovie(gNode->movies.get(idx));
            } else if (action == 'C' || action == 'c') {
                break;
            }
        }
    }
}

void addMovieMenu() {
    std::cin.ignore();
    std::string nama, genreStr, terkaitStr, studio;
    int ep, season;

    std::cout << "==== Tambah Film / Series ====\nNama: ";
    std::getline(std::cin, nama);
    std::cout << "Genre (dipisah koma): ";
    std::getline(std::cin, genreStr);
    std::cout << "Jumlah Episode: ";
    std::cin >> ep;
    std::cout << "Jumlah Season: ";
    std::cin >> season;
    std::cin.ignore();
    std::cout << "Film terkait (dipisah koma): ";
    std::getline(std::cin, terkaitStr);
    std::cout << "Studio: ";
    std::getline(std::cin, studio);

    db.addMovie(nama, splitString(genreStr), ep, season, splitString(terkaitStr), studio);
}

void listAll(bool isSeries) {
    MovieList list = db.getAll(isSeries);
    if (list.empty()) {
        std::cout << "Data kosong.\n";
        return;
    }
    
    std::cout << (isSeries ? "==== Semua Series ====\n" : "==== Semua Film ====\n");
    for (int i = 0; i < list.count; i++) {
        std::cout << i+1 << ". " << list.get(i)->nama << " | " << (list.get(i)->total_rating == 0 ? 0 : list.get(i)->rating) << "\n";
    }
    std::cout << "[C]lose, [R]ate : ";
    char action; std::cin >> action;
    if (action == 'R' || action == 'r') {
        std::cout << "Pilih nomor: ";
        int idx; std::cin >> idx;
        if (idx > 0 && idx <= list.count) {
            rateMovie(list.get(idx-1));
        }
    }
}

int main() {
    while (true) {
        std::cout << "\n==== Wordboxd" << (isAdmin ? " Admin" : "") << " ====\n";
        std::cout << "1. Genre\n2. Search Film / Series\n3. Semua Film\n4. Semua Series\n";
        if (isAdmin) std::cout << "5. Tambah Film / Series\n";
        std::cout << "Pilihan: ";
        
        std::string input;
        std::cin >> input;

        if (input == "0" && !isAdmin) {
            std::cout << "==== Wordboxd Admin ====\nPassphrase: ";
            std::string pass; std::cin >> pass;
            if (pass == "akuadmin727") isAdmin = true;
        } else if (input == "1") genreMenu();
        else if (input == "2") {
            std::cin.ignore();
            std::cout << "Nama: ";
            std::string nama; std::getline(std::cin, nama);
            Movie* m = db.searchMovie(nama);
            if (m) {
                displayMovieDetail(m);
                std::cout << "[C]lose, [R]ate : ";
                char act; std::cin >> act;
                if (act == 'R' || act == 'r') rateMovie(m);
            } else {
                std::cout << "Film tidak ditemukan.\n";
            }
        }
        else if (input == "3") listAll(false);
        else if (input == "4") listAll(true);
        else if (input == "5" && isAdmin) addMovieMenu();
    }
    return 0;
}