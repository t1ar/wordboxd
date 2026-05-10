#include "Database.h"

Database::Database() {
    root = nullptr;
    for (int i = 0; i < HASH_SIZE; i++) {
        genreTable[i] = nullptr;
    }
    initDefaultGenres();
    loadFromFile(); // Otomatis load saat program jalan
}

Database::~Database() {
    // Ruang untuk membersihkan memori heap di sini
}

int Database::hashFunction(std::string key) {
    int sum = 0;
    for (char c : key) sum += c;
    return sum % HASH_SIZE;
}

void Database::initDefaultGenres() {
    std::string defaultGenres[] = {"Action", "Drama", "Fiction", "Comedy", "Animated"};
    for (int i = 0; i < 5; i++) {
        int index = hashFunction(defaultGenres[i]);
        GenreNode* newNode = new GenreNode(defaultGenres[i]);
        newNode->next = genreTable[index];
        genreTable[index] = newNode;
    }
}

GenreNode* Database::getGenre(std::string name) {
    int index = hashFunction(name);
    GenreNode* temp = genreTable[index];
    while (temp != nullptr) {
        if (temp->nama_genre == name) return temp;
        temp = temp->next;
    }
    return nullptr;
}

Movie* Database::insertBST(Movie* node, Movie* newMovie) {
    if (node == nullptr) return newMovie;
    if (newMovie->nama < node->nama) {
        node->left = insertBST(node->left, newMovie);
    } else if (newMovie->nama > node->nama) {
        node->right = insertBST(node->right, newMovie);
    }
    return node;
}

void Database::saveRecursive(Movie* node, std::ofstream& file) {
    if (node == nullptr) return;
    
    saveRecursive(node->left, file);
    
    // Format: Nama|Genre1,Genre2|Ep|Season|Studio|Rating|TotalRate
    file << node->nama << "|";
    for (int i = 0; i < node->genre_names.count; i++) {
        file << node->genre_names.get(i) << (i < node->genre_names.count - 1 ? "," : "");
    }
    file << "|" << node->ep << "|" << node->season << "|" << node->studio << "|" 
         << node->rating << "|" << node->total_rating << "\n";
    
    saveRecursive(node->right, file);
}

void Database::saveToFile() {
    std::ofstream file("database.txt");
    if (file.is_open()) {
        saveRecursive(root, file);
        file.close();
    }
}

// Fungsi helper internal untuk memecah string manual (pengganti split)
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

void Database::loadFromFile() {
    std::ifstream file("database.txt");
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        // Pecah berdasarkan '|'
        StringList parts = splitManual(line, '|');
        if (parts.count < 7) continue;

        std::string nama = parts.get(0);
        StringList genres = splitManual(parts.get(1), ',');
        int ep = std::stoi(parts.get(2));
        int season = std::stoi(parts.get(3));
        std::string studio = parts.get(4);
        float rat = std::stof(parts.get(5));
        int tRat = std::stoi(parts.get(6));

        // Tambahkan ke sistem (tanpa relasi dulu agar tidak error saat load)
        StringList emptyRelated;
        addMovie(nama, genres, ep, season, emptyRelated, studio, true);
        
        // Restore rating asli
        Movie* m = searchMovie(nama);
        if (m) {
            m->rating = rat;
            m->total_rating = tRat;
        }
    }
    file.close();
}

Movie* Database::searchBST(Movie* node, std::string name) {
    if (node == nullptr || node->nama == name) return node;
    if (name < node->nama) return searchBST(node->left, name);
    return searchBST(node->right, name);
}

Movie* Database::searchMovie(std::string name) {
    return searchBST(root, name);
}

bool Database::addMovie(std::string name, StringList genres, int ep, int season, StringList related, std::string studio, bool silent) {
    if (searchMovie(name) != nullptr) return false;

    // Array statis penampung sementara karena film/genre maksimum masuk akal
    GenreNode* validGenres[50]; 
    int validGenresCount = 0;
    
    StringNode* gTemp = genres.head;
    while(gTemp != nullptr) {
        GenreNode* gNode = getGenre(gTemp->data);
        if (gNode != nullptr) validGenres[validGenresCount++] = gNode;
        else std::cout << "Genre " << gTemp->data << " tidak terdaftar.\n";
        gTemp = gTemp->next;
    }

    if (validGenresCount == 0) {
        std::cout << "Semua genre tidak valid, penambahan dibatalkan.\n";
        return false;
    }

    Movie* validRelated[50];
    int validRelatedCount = 0;
    
    StringNode* rTemp = related.head;
    while(rTemp != nullptr) {
        if (!rTemp->data.empty()) {
            Movie* rNode = searchMovie(rTemp->data);
            if (rNode != nullptr) validRelated[validRelatedCount++] = rNode;
            else std::cout << "Film " << rTemp->data << " tidak dapat ditemukan.\n";
        }
        rTemp = rTemp->next;
    }

    Movie* newMovie = new Movie(name, ep, season, studio);
    for (int i = 0; i < validGenresCount; i++) newMovie->genre_names.add(validGenres[i]->nama_genre);
    
    for (int i = 0; i < validRelatedCount; i++) {
        newMovie->related_movies.add(validRelated[i]);
        validRelated[i]->related_movies.add(newMovie);
    }

    for (int i = 0; i < validGenresCount; i++) {
        validGenres[i]->movies.add(newMovie);
    }

    root = insertBST(root, newMovie);
    if (!silent) {
        std::cout << "Data " << name << " berhasil ditambahkan!\n";
        saveToFile(); // Simpan setiap kali ada perubahan
    }
    return true;
}

void Database::getMoviesFiltered(Movie* node, bool isSeries, MovieList& result) {
    if (node != nullptr) {
        getMoviesFiltered(node->left, isSeries, result);
        if ((isSeries && node->ep > 0) || (!isSeries && node->ep == 0)) {
            result.add(node);
        }
        getMoviesFiltered(node->right, isSeries, result);
    }
}

MovieList Database::getAll(bool isSeries) {
    MovieList result;
    getMoviesFiltered(root, isSeries, result);
    return result;
}