/*
Neema Taghipour 8/24/2025
CS 300 - Robert David
Project Two - Course Prerequsites
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>

using namespace std;

// simple course structure
struct Course {
    static const int MAX_PREQS = 8;

    string number;
    string title;
    string prereqs[MAX_PREQS];
    int prereqCount = 0;
};

// hash table with seperate chaining
class CourseHashTable {
public:
    static const size_t TABLE_SIZE = 211;

    CourseHashTable() {
        for (size_t i = 0; i < TABLE_SIZE; ++i) buckets[i] = nullptr;
        itemCount = 0;
    }

    ~CourseHashTable() { clear(); }

    // insert course
    void insert(const Course& c) {
        size_t h = hashKey(c.number);
        Node* cur = buckets[h];

        // replace course if exists
        while (cur) {
            if (cur->key == c.number) {
                cur->value = c;
                return;
            }
            cur = cur->next;
        }

        // prepend new node
        Node* n = new Node(c.number, c, buckets[h]);
        buckets[h] = n;
        ++itemCount;
    }

    // find course
    const Course* find(const string& key) const {
        size_t h = hashKey(key);
        Node* cur = buckets[h];
        while (cur) {
            if (cur->key == key) return &cur->value;
            cur = cur->next;
        }
        return nullptr;
    }

    template <typename F>
    void forEach(F fn) const {
        for (size_t i = 0; i < TABLE_SIZE; ++i) {
            Node* cur = buckets[i];
            while (cur) {
                fn(cur->value);
                cur = cur->next;
            }
        }
    }

    // count courses
    size_t size() const { return itemCount; }

    void clear() {
        for (size_t i = 0; i < TABLE_SIZE; ++i) {
            Node* cur = buckets[i];
            while (cur) {
                Node* nxt = cur->next;
                delete cur;
                cur = nxt;
            }
            buckets[i] = nullptr;
        }
        itemCount = 0;
    }

private:
    struct Node {
        string key;
        Course value;
        Node* next;
        Node(const string& k, const Course& v, Node* n) : key(k), value(v), next(n) {}
    };

    Node* buckets[TABLE_SIZE];
    size_t itemCount;

    // hash
    static size_t hashKey(const string& s) {
        size_t h = 2166136261u; // FNV-like start, fits in size_t
        for (unsigned char c : s) {
            h ^= c;
            h *= 16777619u; // FNV prime, fits in size_t
        }
        return h % TABLE_SIZE;
    }
};

// removes spaces
static inline string trim(const string& in) {
    size_t a = 0, b = in.size();
    while (a < b && isspace(static_cast<unsigned char>(in[a]))) ++a;
    while (b > a && isspace(static_cast<unsigned char>(in[b - 1]))) --b;
    return in.substr(a, b - a);
}


static inline string lowerCopy(string s) {
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

// load csv
bool loadCoursesFromCSV(const string& filename, CourseHashTable& table) {
    ifstream fin(filename);
    if (!fin) {
        cerr << "Error: Could not open file \"" << filename << "\".\n";
        return false;
    }

    // clear existing datz
    table.clear();

    string line;
    size_t lineNum = 0;

    while (getline(fin, line)) {
        ++lineNum;
    
        bool onlySpaces = true;
        for (char ch : line) if (!isspace(static_cast<unsigned char>(ch))) { onlySpaces = false; break; }
        if (onlySpaces) continue;

        // parse
        string fields[32];
        int fcount = 0;

        string field;
        stringstream ss(line);
        while (getline(ss, field, ',')) {
            if (fcount < 32) fields[fcount++] = trim(field);
        }

        if (fcount < 2) {
            cerr << "Warning: Line " << lineNum << " malformed (needs at least course and title). Skipped.\n";
            continue;
        }

        Course c;
        c.number = fields[0];
        c.title  = fields[1];
        c.prereqCount = 0;

        for (int i = 2; i < fcount; ++i) {
            if (!fields[i].empty() && c.prereqCount < Course::MAX_PREQS) {
                c.prereqs[c.prereqCount++] = fields[i];
            }
        }

        table.insert(c);
    }

    return true;
}

// print course information for 1 course
void printCourseInfo(const CourseHashTable& table, const string& courseNumRaw) {
    string courseNum = trim(courseNumRaw);
    if (courseNum.empty()) {
        cerr << "Error: Course number cannot be empty.\n";
        return;
    }

    const Course* c = table.find(courseNum);
    if (!c) {
        cerr << "Error: Course \"" << courseNum << "\" not found.\n";
        return;
    }

    cout << c->number << ": " << c->title << "\n";
    cout << "Prerequisites: ";
    if (c->prereqCount == 0) {
        cout << "None\n";
    } else {
        for (int i = 0; i < c->prereqCount; ++i) {
            const string& preqNum = c->prereqs[i];
            const Course* pc = table.find(preqNum);
            if (pc) {
                cout << pc->number << " (" << pc->title << ")";
            } else {
                // If a prerequisite isn’t in the table, print its number anyway.
                cout << preqNum;
            }
            if (i + 1 < c->prereqCount) cout << ", ";
        }
        cout << "\n";
    }
}

// print courses numerically sorted
void printAllCoursesSorted(const CourseHashTable& table) {
    size_t n = table.size();
    if (n == 0) {
        cerr << "Error: No courses loaded. Choose option 1 first.\n";
        return;
    }

    // first pass
    const Course** arr = new const Course*[n];
    size_t idx = 0;
    table.forEach([&](const Course& c) {
        arr[idx++] = &c;
    });

    // insertion sort
    for (size_t i = 1; i < n; ++i) {
        const Course* key = arr[i];
        size_t j = i;
        while (j > 0 && arr[j - 1]->number > key->number) {
            arr[j] = arr[j - 1];
            --j;
        }
        arr[j] = key;
    }

    // print course # and title
    cout << "Course List (alphanumeric):\n";
    for (size_t i = 0; i < n; ++i) {
        cout << arr[i]->number << " - " << arr[i]->title << "\n";
    }

    delete[] arr;
}

// msin function
int main() {
    CourseHashTable table;
    bool dataLoaded = false;

    while (true) {
        cout << "\n===== Course Planner =====\n";
        cout << "  1. Load course data\n";
        cout << "  2. Print course list (alphanumeric)\n";
        cout << "  3. Print course info (title + prerequisites)\n";
        cout << "  9. Exit\n";
        cout << "Choose an option: ";

        string choiceStr;
        if (!getline(cin, choiceStr)) break;
        choiceStr = trim(choiceStr);
        if (choiceStr.empty()) {
            cerr << "Invalid selection. Please enter 1, 2, 3, or 9.\n";
            continue;
        }

        int choice = -1;
        // simple numeric parse
        {
            bool ok = true;
            for (char ch : choiceStr) {
                if (!isdigit(static_cast<unsigned char>(ch))) { ok = false; break; }
            }
            if (ok) choice = stoi(choiceStr);
        }

        switch (choice) {
            case 1: {
                cout << "Enter course data filename (CSV): ";
                string filename;
                if (!getline(cin, filename)) {
                    cerr << "Error reading filename.\n";
                    break;
                }
                filename = trim(filename);
                if (filename.empty()) {
                    cerr << "Error: Filename cannot be empty.\n";
                    break;
                }
                bool ok = loadCoursesFromCSV(filename, table);
                if (ok) {
                    dataLoaded = true;
                    cout << "Loaded " << table.size() << " course(s) from \"" << filename << "\".\n";
                } else {
                    dataLoaded = false;
                }
                break;
            }

            case 2: {
                if (!dataLoaded) {
                    cerr << "Error: Load data first (option 1).\n";
                } else {
                    printAllCoursesSorted(table);
                }
                break;
            }

            case 3: {
                if (!dataLoaded) {
                    cerr << "Error: Load data first (option 1).\n";
                } else {
                    cout << "Enter a course number (e.g., CSCI200): ";
                    string num;
                    if (!getline(cin, num)) {
                        cerr << "Error reading course number.\n";
                        break;
                    }
                    printCourseInfo(table, num);
                }
                break;
            }

            case 9: {
                cout << "Goodbye!\n";
                return 0;
            }

            default:
                cerr << "Invalid selection. Please enter 1, 2, 3, or 9.\n";
                break;
        }
    }

    return 0;
}