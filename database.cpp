#include "database.h"

Database::Database() {
    root = nullptr;
    // inisialisasi semua indeks tabel hash dengan nullptr agar aman
    for (int i = 0; i < HASH_SIZE; i++) {
        genreTable[i] = nullptr;
    }
    initDefaultGenres();
    loadFromFile(); // otomatis load saat program jalan
}

Database::~Database() {
    // ruang untuk membersihkan memori heap di sini
}

// cara kerja hash function:
// menjumlahkan nilai ascii dari setiap huruf pada nama genre,
// lalu di-modulo (sisa bagi) dengan ukuran tabel (hash_size).
// hasilnya akan selalu berupa angka dari 0 sampai hash_size-1 sebagai indeks array.
int Database::hashFunction(std::string key) {
    int sum = 0;
    for (char c : key) sum += c;
    return sum % HASH_SIZE;
}

// memasukkan data genre dasar ke dalam hash table
void Database::initDefaultGenres() {
    std::string defaultGenres[] = {"Action", "Drama", "Fiction", "Comedy", "Animated"};
    for (int i = 0; i < 5; i++) {
        int index = hashFunction(defaultGenres[i]);
        GenreNode* newNode = new GenreNode(defaultGenres[i]);
        // ini adalah teknik chaining: node baru disisipkan di awal (head) dari indeks tabel hash tersebut
        newNode->next = genreTable[index];
        genreTable[index] = newNode;
    }
}

// mencari genre di dalam hash table
GenreNode* Database::getGenre(std::string name) {
    int index = hashFunction(name);
    GenreNode* temp = genreTable[index];
    // mencari (traversal) di dalam chain linked list pada indeks tersebut jika terjadi collision
    while (temp != nullptr) {
        if (temp->nama_genre == name) return temp;
        temp = temp->next;
    }
    return nullptr;
}

// cara kerja insert binary search tree (bst):
// fungsi ini akan menelusuri tree dari root.
// ia membandingkan nama film menggunakan operator '<' atau '>' (leksikografis/alfabet).
// jika nama baru lebih kecil dari node saat ini, ia turun ke cabang kiri (left).
// jika lebih besar, ia turun ke cabang kanan (right).
// ini dilakukan terus secara rekursif sampai menemukan cabang yang kosong (nullptr), lalu diletakkan di sana.
Movie* Database::insertBST(Movie* node, Movie* newMovie) {
    if (node == nullptr) return newMovie;
    if (newMovie->nama < node->nama) {
        node->left = insertBST(node->left, newMovie);
    } else if (newMovie->nama > node->nama) {
        node->right = insertBST(node->right, newMovie);
    }
    return node;
}

// cara kerja traversal tree (in-order):
// menelusuri ke kiri dulu sampai mentok, lalu mencetak/memproses node saat ini, baru menelusuri ke kanan.
// sifat in-order traversal pada bst akan menghasilkan data yang berurutan sesuai alfabet nama film.
void Database::saveRecursive(Movie* node, std::ofstream& file) {
    if (node == nullptr) return;
    
    saveRecursive(node->left, file);
    
    // format data: nama|genre1,genre2|ep|season|studio|rating|totalrate
    file << node->nama << "|";
    for (int i = 0; i < node->genre_names.count; i++) {
        file << node->genre_names.get(i) << (i < node->genre_names.count - 1 ? "," : "");
    }
    file << "|" << node->ep << "|" << node->season << "|" << node->studio << "|" 
         << node->rating << "|" << node->total_rating;
         
    // tambahan: tambahkan kolom pembatas '|' baru untuk menyimpan film terkait
    file << "|";
    for (int i = 0; i < node->related_movies.count; i++) {
        file << node->related_movies.get(i)->nama << (i < node->related_movies.count - 1 ? "," : "");
    }
    file << "\n";
    
    saveRecursive(node->right, file);
}

// membuka file dan memanggil penyimpan rekursif bst
void Database::saveToFile() {
    std::ofstream file("database.txt");
    if (file.is_open()) {
        saveRecursive(root, file);
        file.close();
    }
}

// fungsi helper internal untuk memecah string manual berdasarkan karakter pemisah
StringList splitManual(std::string str, char delimiter) {
    StringList res;
    std::string temp = "";
    for (char c : str) {
        if (c == delimiter) {
            res.add(temp);
            temp = "";
        } else temp += c;
    }
    if (!temp.empty()) res.add(temp);
    return res;
}

// membaca file dan membangun ulang tree dan graf
void Database::loadFromFile() {
    std::ifstream file("database.txt");
    if (!file.is_open()) return;

    std::string line;
    
    // === pass 1: bangun semua node film terlebih dahulu ke dalam tree ===
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        StringList parts = splitManual(line, '|');
        if (parts.count < 7) continue;

        std::string nama = parts.get(0);
        StringList genres = splitManual(parts.get(1), ',');
        int ep = std::stoi(parts.get(2));
        int season = std::stoi(parts.get(3));
        std::string studio = parts.get(4);
        float rat = std::stof(parts.get(5));
        int tRat = std::stoi(parts.get(6));

        // selalu kirim relasi kosong di pass 1 ini agar aman
        StringList emptyRelated;
        addMovie(nama, genres, ep, season, emptyRelated, studio, true);
        
        Movie* m = searchMovie(nama);
        if (m) {
            m->rating = rat;
            m->total_rating = tRat;
        }
    }
    
    // === pass 2: setelah semua node jadi, baca file lagi untuk menyambungkan graf-nya ===
    // reset posisi pembacaan file kembali ke baris paling atas
    file.clear();
    file.seekg(0, std::ios::beg);
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        StringList parts = splitManual(line, '|');
        // jika ada minimal 8 kolom, berarti film ini punya relasi
        if (parts.count >= 8) {
            std::string namaUtama = parts.get(0);
            Movie* mUtama = searchMovie(namaUtama);
            if (mUtama == nullptr) continue; // untuk keamanan
            
            // pecah daftar film terkait di kolom ke-8
            StringList relatedNames = splitManual(parts.get(7), ',');
            StringNode* temp = relatedNames.head;
            
            while (temp != nullptr) {
                if (!temp->data.empty()) {
                    Movie* mTerkait = searchMovie(temp->data);
                    if (mTerkait != nullptr) {
                        // cek manual apakah relasi sudah ada untuk mencegah duplikasi jabat tangan
                        bool sudahAda = false;
                        for (int i = 0; i < mUtama->related_movies.count; i++) {
                            if (mUtama->related_movies.get(i)->nama == mTerkait->nama) {
                                sudahAda = true;
                                break;
                            }
                        }
                        
                        // jika belum berteman, tambahkan alamat memori temannya ke buku kontak
                        if (!sudahAda) {
                            mUtama->related_movies.add(mTerkait);
                        }
                    }
                }
                temp = temp->next;
            }
        }
    }
    file.close();
}

// cara kerja pencarian di binary search tree (bst):
// sama seperti cara insert, data dicari dengan membandingkan alfabet nama film.
// secara rekursif bergerak ke cabang kiri jika string dicari lebih kecil, atau kanan jika lebih besar.
// sangat cepat karena setiap turun 1 tingkat, kita mengeliminasi setengah kemungkinan jalur (time complexity o(log n)).
Movie* Database::searchBST(Movie* node, std::string name) {
    if (node == nullptr || node->nama == name) return node;
    if (name < node->nama) return searchBST(node->left, name);
    return searchBST(node->right, name);
}

// fungsi pembungkus (wrapper) untuk memanggil pencarian dari root tree
Movie* Database::searchMovie(std::string name) {
    return searchBST(root, name);
}

// proses utama menambahkan film ke dalam bst dan menghubungkan simpul graf
bool Database::addMovie(std::string name, StringList genres, int ep, int season, StringList related, std::string studio, bool silent) {
    if (searchMovie(name) != nullptr) return false;

    // array statis penampung sementara node genre karena maksimal wajar tidak banyak
    GenreNode* validGenres[50]; 
    int validGenresCount = 0;
    
    // mengecek apakah genre yang diinput user ada di dalam hash table
    StringNode* gTemp = genres.head;
    while(gTemp != nullptr) {
        GenreNode* gNode = getGenre(gTemp->data);
        if (gNode != nullptr) validGenres[validGenresCount++] = gNode;
        else std::cout << "genre " << gTemp->data << " tidak terdaftar.\n";
        gTemp = gTemp->next;
    }

    // jika tidak ada satupun genre yang cocok di tabel hash, batalkan
    if (validGenresCount == 0) {
        std::cout << "semua genre tidak valid, penambahan dibatalkan.\n";
        return false;
    }

    // array statis penampung relasi graf sementara
    Movie* validRelated[50];
    int validRelatedCount = 0;
    
    // mencari film yang ingin dihubungkan di dalam tree
    StringNode* rTemp = related.head;
    while(rTemp != nullptr) {
        if (!rTemp->data.empty()) {
            Movie* rNode = searchMovie(rTemp->data);
            if (rNode != nullptr) validRelated[validRelatedCount++] = rNode;
            else std::cout << "film " << rTemp->data << " tidak dapat ditemukan.\n";
        }
        rTemp = rTemp->next;
    }

    // membuat simpul/node data film baru
    Movie* newMovie = new Movie(name, ep, season, studio);
    for (int i = 0; i < validGenresCount; i++) newMovie->genre_names.add(validGenres[i]->nama_genre);
    
    // cara kerja pembentukan edge (garis/hubungan) pada graph:
    // menghubungkan film baru (newmovie) ke film lama yang sudah dicari (rnode/validrelated).
    // karena ini graf tak berarah (undirected), maka harus di-link dari dua sisi:
    // sisi 1: list relasi newmovie ditambahkan film rnode.
    // sisi 2: list relasi rnode ditambahkan film newmovie.
    for (int i = 0; i < validRelatedCount; i++) {
        newMovie->related_movies.add(validRelated[i]);
        validRelated[i]->related_movies.add(newMovie);
    }

    // mendaftarkan film baru ke dalam masing-masing list genre di dalam hash table
    for (int i = 0; i < validGenresCount; i++) {
        validGenres[i]->movies.add(newMovie);
    }

    // terakhir, menyisipkan film baru ke dalam struktur binary search tree
    root = insertBST(root, newMovie);
    
    if (!silent) {
        std::cout << "data " << name << " berhasil ditambahkan!\n";
        saveToFile(); // simpan setiap kali ada perubahan data di tree
    }
    return true;
}

// rekursif in-order traversal pada tree untuk mengumpulkan list film, dengan filter apakah itu series atau bukan
void Database::getMoviesFiltered(Movie* node, bool isSeries, MovieList& result) {
    if (node != nullptr) {
        getMoviesFiltered(node->left, isSeries, result);
        // jika series maka punya ep > 0, jika film lepas maka ep == 0
        if ((isSeries && node->ep > 0) || (!isSeries && node->ep == 0)) {
            result.add(node);
        }
        getMoviesFiltered(node->right, isSeries, result);
    }
}

// fungsi pemanggil traversal filter di atas dari root bst
MovieList Database::getAll(bool isSeries) {
    MovieList result;
    getMoviesFiltered(root, isSeries, result);
    return result;
}