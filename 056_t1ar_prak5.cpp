#include <iostream>
#include <sstream>
#include "database.h"

// instansiasi global untuk objek database agar bisa diakses semua fungsi menu
Database db;
bool isAdmin = false;

// trimming manual untuk membersihkan spasi berlebih di awal dan akhir input string user (pengganti algorithm trim bawaan)
std::string trimString(std::string str) {
    int start = 0;
    while(start < str.length() && (str[start] == ' ' || str[start] == '\t')) start++;
    int end = str.length() - 1;
    while(end >= 0 && (str[end] == ' ' || str[end] == '\t')) end--;
    if (start > end) return "";
    return str.substr(start, end - start + 1);
}

// split string berdasarkan koma menjadi custom stringlist, berguna untuk memecah input list genre dan graf relasi user
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

// menampilkan seluruh informasi terkait node film termasuk graf (film terkait/adjacency list-nya)
void displayMovieDetail(Movie* m) {
    std::cout << "\n---- " << m->nama << " ----\ngenre: ";
    for (int i = 0; i < m->genre_names.count; i++) {
        std::cout << m->genre_names.get(i) << (i < m->genre_names.count - 1 ? ", " : "");
    }
    std::cout << "\njumlah ep: " << (m->ep == 0 ? "-" : std::to_string(m->ep));
    std::cout << "\njumlah season: " << (m->season == 0 ? "-" : std::to_string(m->season));
    
    // ini menampilkan edge/koneksi graf dari satu node ke node lain yang berelasi
    std::cout << "\nfilm yang terkait:\n";
    if (m->related_movies.empty()) std::cout << "- tidak ada\n";
    else {
        for (int i = 0; i < m->related_movies.count; i++) {
            std::cout << "- " << m->related_movies.get(i)->nama << "\n";
        }
    }
    std::cout << "studio: " << m->studio << "\n";
    std::cout << "rating: " << (m->total_rating == 0 ? 0 : m->rating) << "\n";
}

// meminta input perhitungan rating ke dalam objek
void rateMovie(Movie* m) {
    std::cout << "==== rate " << m->nama << " ====\nrating: ";
    float rate; std::cin >> rate;
    m->addRating(rate);
    std::cout << "rating terbaru: " << m->rating << "\n";
    db.saveToFile(); // penting: simpan update rating ini kembali dari bst ke file fisik
}

// antarmuka menu yang mengambil data dari hash table genre
void genreMenu() {
    std::string gList[] = {"Action", "Drama", "Fiction", "Comedy", "Animated"};
    std::cout << "==== pilih genre ====\n";
    for (int i = 0; i < 5; i++) {
        std::cout << i+1 << ". " << gList[i] << "\n";
    }
    std::cout << "pilih: ";
    int pil; std::cin >> pil;

    if (pil > 0 && pil <= 5) {
        // mengambil node pada hash table sesuai dengan nama genrenya
        GenreNode* gNode = db.getGenre(gList[pil-1]);
        if (gNode->movies.empty()) {
            std::cout << "belum ada film di genre ini.\n";
            return;
        }

        int idx = 0;
        char action;
        // navigasi list film yang berasosiasi dalam genre ini
        while (true) {
            displayMovieDetail(gNode->movies.get(idx));
            std::cout << "[n]ext, [p]revious, [c]lose, [r]ate : ";
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

// menu untuk menyisipkan node baru ke dalam struktur bst dan graf
void addMovieMenu() {
    std::cin.ignore();
    std::string nama, genreStr, terkaitStr, studio;
    int ep, season;

    std::cout << "==== tambah film / series ====\nnama: ";
    std::getline(std::cin, nama);
    std::cout << "genre (dipisah koma): ";
    std::getline(std::cin, genreStr);
    std::cout << "jumlah episode: ";
    std::cin >> ep;
    std::cout << "jumlah season: ";
    std::cin >> season;
    std::cin.ignore();
    // memfasilitasi relasi pembentukan adjacency list graf
    std::cout << "film terkait (dipisah koma): ";
    std::getline(std::cin, terkaitStr);
    std::cout << "studio: ";
    std::getline(std::cin, studio);

    // kirim input yang dipecah ke dalam metode graf dan tree di class database
    db.addMovie(nama, splitString(genreStr), ep, season, splitString(terkaitStr), studio);
}

// mengambil hasil tree traversal untuk ditampilkan di CLI berdasarkan jenis (series atau bukan)
void listAll(bool isSeries) {
    MovieList list = db.getAll(isSeries);
    if (list.empty()) {
        std::cout << "data kosong.\n";
        return;
    }
    
    std::cout << (isSeries ? "==== semua series ====\n" : "==== semua film ====\n");
    for (int i = 0; i < list.count; i++) {
        std::cout << i+1 << ". " << list.get(i)->nama << " | " << (list.get(i)->total_rating == 0 ? 0 : list.get(i)->rating) << "\n";
    }
    std::cout << "[c]lose, [r]ate : ";
    char action; std::cin >> action;
    if (action == 'R' || action == 'r') {
        std::cout << "pilih nomor: ";
        int idx; std::cin >> idx;
        if (idx > 0 && idx <= list.count) {
            rateMovie(list.get(idx-1));
        }
    }
}

// loop antarmuka program berjalan
int main() {
    while (true) {
        std::cout << "\n==== wordboxd" << (isAdmin ? " admin" : "") << " ====\n";
        std::cout << "1. genre\n2. search film / series\n3. semua film\n4. semua series\n";
        if (isAdmin) std::cout << "5. tambah film / series\n";
        std::cout << "pilihan: ";
        
        std::string input;
        std::cin >> input;

        // pintu belakang (backdoor) ke menu admin
        if (input == "0" && !isAdmin) {
            std::cout << "==== wordboxd admin ====\npassphrase: ";
            std::string pass; std::cin >> pass;
            if (pass == "akuadmin727") isAdmin = true;
        } else if (input == "1") genreMenu();
        else if (input == "2") {
            std::cin.ignore();
            std::cout << "nama: ";
            std::string nama; std::getline(std::cin, nama);
            // eksekusi pencarian o(log n) ke dalam binary search tree (bst)
            Movie* m = db.searchMovie(nama);
            if (m) {
                displayMovieDetail(m);
                std::cout << "[c]lose, [r]ate : ";
                char act; std::cin >> act;
                if (act == 'R' || act == 'r') rateMovie(m);
            } else {
                std::cout << "film tidak ditemukan.\n";
            }
        }
        else if (input == "3") listAll(false);
        else if (input == "4") listAll(true);
        else if (input == "5" && isAdmin) addMovieMenu();
    }
    return 0;
}