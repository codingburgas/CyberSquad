#include "data.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

 // ═══════════════════════════════════════════════════════════════
 //  Internal helpers (not exposed in header)
 // ═══════════════════════════════════════════════════════════════

 // Convert a string to lower-case in-place
static void str_toLower(std::string& s)
{
    for (char& c : s)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

// Trim leading/trailing whitespace from a string
static std::string str_trim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";
    return s.substr(start, end - start + 1);
}

// Split a string by a delimiter character
static std::vector<std::string> str_split(const std::string& s, char delim)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(s);
    while (std::getline(stream, token, delim))
        tokens.push_back(token);
    return tokens;
}

// Find a class record by label; returns nullptr if not found
static ClassRecord* findClass(DataStore& store, const std::string& label)
{
    for (ClassRecord& cr : store.classes)
        if (cr.label == label)
            return &cr;
    return nullptr;
}

// Remove a student ID from all class records
static void removeFromAllClasses(DataStore& store, int studentId)
{
    for (ClassRecord& cr : store.classes)
    {
        auto it = std::find(cr.studentIds.begin(), cr.studentIds.end(), studentId);
        if (it != cr.studentIds.end())
            cr.studentIds.erase(it);
    }
}

// Encode a SubjectGrade to a compact string  "code:value:note"
static std::string encodeGrade(const SubjectGrade& sg)
{
    std::string encoded = data_subjectCode(sg.subject) + ":"
        + std::to_string(sg.value) + ":"
        + sg.note;
    return encoded;
}

// Decode a SubjectGrade from "code:value:note"
static SubjectGrade decodeGrade(const std::string& token)
{
    SubjectGrade sg;
    sg.value = 2.0;
    auto parts = str_split(token, ':');
    if (parts.size() >= 1) sg.subject = data_subjectFromCode(parts[0]);
    if (parts.size() >= 2) sg.value = std::stod(parts[1]);
    if (parts.size() >= 3) sg.note = parts[2];
    return sg;
}

// Encode all grades to a semicolon-delimited string
static std::string encodeAllGrades(const std::vector<SubjectGrade>& grades)
{
    std::string result;
    for (size_t i = 0; i < grades.size(); ++i)
    {
        if (i > 0) result += ";";
        result += encodeGrade(grades[i]);
    }
    return result;
}

// Decode all grades from a semicolon-delimited string
static std::vector<SubjectGrade> decodeAllGrades(const std::string& s)
{
    std::vector<SubjectGrade> grades;
    if (s.empty()) return grades;
    auto tokens = str_split(s, ';');
    for (const auto& token : tokens)
    {
        if (!token.empty())
            grades.push_back(decodeGrade(token));
    }
    return grades;
}

// ═══════════════════════════════════════════════════════════════
//  DataStore – Initialisation
// ═══════════════════════════════════════════════════════════════

DataStore data_createStore()
{
    DataStore store;
    store.nextId = 1;
    return store;
}

// ═══════════════════════════════════════════════════════════════
//  CRUD – Students
// ═══════════════════════════════════════════════════════════════

int data_addStudent(DataStore& store, const Student& student)
{
    Student newStudent = student;
    newStudent.id = data_generateId(store);
    newStudent.isActive = true;

    // Hash the EGN before storing
    if (!newStudent.egn.empty())
        newStudent.egn = data_hashEgn(newStudent.egn);

    store.students.push_back(newStudent);

    // Also register the class if needed
    if (!newStudent.classLabel.empty())
    {
        data_addClass(store, newStudent.classLabel);
        ClassRecord* cr = findClass(store, newStudent.classLabel);
        if (cr) cr->studentIds.push_back(newStudent.id);
    }

    return newStudent.id;
}

Student* data_findById(DataStore& store, int id)
{
    for (Student& s : store.students)
        if (s.id == id && s.isActive)
            return &s;
    return nullptr;
}

std::vector<Student*> data_findByLastName(DataStore& store,
    const std::string& lastName)
{
    std::vector<Student*> results;
    std::string query = lastName;
    str_toLower(query);

    for (Student& s : store.students)
    {
        if (!s.isActive) continue;
        std::string lname = s.lastName;
        str_toLower(lname);
        if (lname.find(query) != std::string::npos)
            results.push_back(&s);
    }
    return results;
}

std::vector<Student*> data_findByClass(DataStore& store,
    const std::string& classLabel)
{
    std::vector<Student*> results;
    for (Student& s : store.students)
    {
        if (!s.isActive) continue;
        if (s.classLabel == classLabel)
            results.push_back(&s);
    }
    return results;
}

bool data_updateStudent(DataStore& store, const Student& updated)
{
    for (Student& s : store.students)
    {
        if (s.id == updated.id)
        {
            // Preserve hashed EGN if caller sends empty string
            std::string egn = updated.egn.empty() ? s.egn : updated.egn;

            // If class changed, update ClassRecord membership
            if (s.classLabel != updated.classLabel)
            {
                removeFromAllClasses(store, s.id);
                data_addClass(store, updated.classLabel);
                ClassRecord* cr = findClass(store, updated.classLabel);
                if (cr) cr->studentIds.push_back(s.id);
            }

            s = updated;
            s.egn = egn;
            s.isActive = true;
            return true;
        }
    }
    return false;
}

bool data_deleteStudent(DataStore& store, int id)
{
    for (Student& s : store.students)
    {
        if (s.id == id && s.isActive)
        {
            s.isActive = false;
            removeFromAllClasses(store, id);
            return true;
        }
    }
    return false;
}

std::vector<Student*> data_getAllActive(DataStore& store)
{
    std::vector<Student*> results;
    for (Student& s : store.students)
        if (s.isActive)
            results.push_back(&s);
    return results;
}

// ═══════════════════════════════════════════════════════════════
//  Grades
// ═══════════════════════════════════════════════════════════════

bool data_setGrade(DataStore& store, int studentId,
    Subject subject, double value,
    const std::string& note)
{
    if (!data_isValidGrade(value)) return false;

    Student* s = data_findById(store, studentId);
    if (!s) return false;

    // Overwrite if subject already exists
    for (SubjectGrade& sg : s->grades)
    {
        if (sg.subject == subject)
        {
            sg.value = value;
            sg.note = note;
            return true;
        }
    }

    // Otherwise add new entry
    SubjectGrade sg;
    sg.subject = subject;
    sg.value = value;
    sg.note = note;
    s->grades.push_back(sg);
    return true;
}

double data_getGrade(const Student& student, Subject subject)
{
    for (const SubjectGrade& sg : student.grades)
        if (sg.subject == subject)
            return sg.value;
    return -1.0; // Not found
}

bool data_removeGrade(DataStore& store, int studentId, Subject subject)
{
    Student* s = data_findById(store, studentId);
    if (!s) return false;

    auto it = std::remove_if(s->grades.begin(), s->grades.end(),
        [subject](const SubjectGrade& sg) { return sg.subject == subject; });

    if (it == s->grades.end()) return false;
    s->grades.erase(it, s->grades.end());
    return true;
}

// ═══════════════════════════════════════════════════════════════
//  Class Management
// ═══════════════════════════════════════════════════════════════

bool data_addClass(DataStore& store, const std::string& label)
{
    if (label.empty()) return false;
    if (findClass(store, label) != nullptr) return false; // Already exists

    ClassRecord cr;
    cr.label = label;
    store.classes.push_back(cr);
    return true;
}

bool data_assignToClass(DataStore& store, int studentId,
    const std::string& classLabel)
{
    Student* s = data_findById(store, studentId);
    if (!s) return false;

    // Remove from old class
    removeFromAllClasses(store, studentId);

    // Add to new class
    data_addClass(store, classLabel); // No-op if exists
    ClassRecord* cr = findClass(store, classLabel);
    if (!cr) return false;

    cr->studentIds.push_back(studentId);
    s->classLabel = classLabel;
    return true;
}

std::vector<std::string> data_getAllClassLabels(const DataStore& store)
{
    std::vector<std::string> labels;
    for (const ClassRecord& cr : store.classes)
        labels.push_back(cr.label);
    std::sort(labels.begin(), labels.end());
    return labels;
}

// ═══════════════════════════════════════════════════════════════
//  Persistence – File I/O
// ═══════════════════════════════════════════════════════════════
/*
 * File format (CSV-like, pipe-delimited):
 *
 * Line 1:  NEXTID|<value>
 * Lines 2+: STUDENT|id|firstName|lastName|egn(hashed)|gender|yearGroup
 *                  |classLabel|isActive|grades(semicolon-separated)
 *
 * Classes are rebuilt from student data on load.
 */

bool data_saveToFile(const DataStore& store, const std::string& filepath)
{
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    // Write meta
    file << "NEXTID|" << store.nextId << "\n";

    for (const Student& s : store.students)
    {
        file << "STUDENT"
            << "|" << s.id
            << "|" << s.firstName
            << "|" << s.lastName
            << "|" << s.egn
            << "|" << static_cast<int>(s.gender)
            << "|" << static_cast<int>(s.yearGroup)
            << "|" << s.classLabel
            << "|" << (s.isActive ? 1 : 0)
            << "|" << encodeAllGrades(s.grades)
            << "\n";
    }

    file.close();
    return true;
}

bool data_loadFromFile(DataStore& store, const std::string& filepath)
{
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    store.students.clear();
    store.classes.clear();
    store.nextId = 1;

    std::string line;
    while (std::getline(file, line))
    {
        line = str_trim(line);
        if (line.empty()) continue;

        auto parts = str_split(line, '|');
        if (parts.empty()) continue;

        if (parts[0] == "NEXTID" && parts.size() >= 2)
        {
            store.nextId = std::stoi(parts[1]);
        }
        else if (parts[0] == "STUDENT" && parts.size() >= 10)
        {
            Student s;
            s.id = std::stoi(parts[1]);
            s.firstName = parts[2];
            s.lastName = parts[3];
            s.egn = parts[4];
            s.gender = static_cast<Gender>(std::stoi(parts[5]));
            s.yearGroup = static_cast<Grade>(std::stoi(parts[6]));
            s.classLabel = parts[7];
            s.isActive = (parts[8] == "1");
            s.grades = decodeAllGrades(parts[9]);
            store.students.push_back(s);

            // Rebuild class membership
            if (!s.classLabel.empty() && s.isActive)
            {
                data_addClass(store, s.classLabel);
                ClassRecord* cr = findClass(store, s.classLabel);
                if (cr) cr->studentIds.push_back(s.id);
            }
        }
    }

    file.close();
    return true;
}

// ═══════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════

std::string data_subjectName(Subject s)
{
    switch (s)
    {
    case Subject::MATHEMATICS: return "Mathematics";
    case Subject::BULGARIAN:   return "Bulgarian";
    case Subject::ENGLISH:     return "English";
    case Subject::HISTORY:     return "History";
    case Subject::GEOGRAPHY:   return "Geography";
    case Subject::PHYSICS:     return "Physics";
    case Subject::CHEMISTRY:   return "Chemistry";
    case Subject::BIOLOGY:     return "Biology";
    case Subject::INFORMATICS: return "Informatics";
    case Subject::PHYSICAL_ED: return "Physical Ed.";
    default:                   return "Unknown";
    }
}

std::string data_subjectCode(Subject s)
{
    switch (s)
    {
    case Subject::MATHEMATICS: return "MAT";
    case Subject::BULGARIAN:   return "BUL";
    case Subject::ENGLISH:     return "ENG";
    case Subject::HISTORY:     return "HIS";
    case Subject::GEOGRAPHY:   return "GEO";
    case Subject::PHYSICS:     return "PHY";
    case Subject::CHEMISTRY:   return "CHE";
    case Subject::BIOLOGY:     return "BIO";
    case Subject::INFORMATICS: return "INF";
    case Subject::PHYSICAL_ED: return "PHE";
    default:                   return "UNK";
    }
}

Subject data_subjectFromCode(const std::string& code)
{
    if (code == "MAT") return Subject::MATHEMATICS;
    if (code == "BUL") return Subject::BULGARIAN;
    if (code == "ENG") return Subject::ENGLISH;
    if (code == "HIS") return Subject::HISTORY;
    if (code == "GEO") return Subject::GEOGRAPHY;
    if (code == "PHY") return Subject::PHYSICS;
    if (code == "CHE") return Subject::CHEMISTRY;
    if (code == "BIO") return Subject::BIOLOGY;
    if (code == "INF") return Subject::INFORMATICS;
    if (code == "PHE") return Subject::PHYSICAL_ED;
    return Subject::MATHEMATICS;
}

std::string data_genderName(Gender g)
{
    switch (g)
    {
    case Gender::MALE:   return "Male";
    case Gender::FEMALE: return "Female";
    case Gender::OTHER:  return "Other";
    default:             return "Unknown";
    }
}

std::string data_gradeName(Grade g)
{
    switch (g)
    {
    case Grade::FIRST:  return "8th grade";
    case Grade::SECOND: return "9th grade";
    case Grade::THIRD:  return "10th grade";
    case Grade::FOURTH: return "11th grade";
    default:            return "Unknown";
    }
}

std::vector<Subject> data_allSubjects()
{
    return {
        Subject::MATHEMATICS,
        Subject::BULGARIAN,
        Subject::ENGLISH,
        Subject::HISTORY,
        Subject::GEOGRAPHY,
        Subject::PHYSICS,
        Subject::CHEMISTRY,
        Subject::BIOLOGY,
        Subject::INFORMATICS,
        Subject::PHYSICAL_ED
    };
}

// Simple DJB2-based hash for EGN – not cryptographic, just obscures raw data
std::string data_hashEgn(const std::string& egn)
{
    unsigned long hash = 5381;
    for (char c : egn)
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    // Return as hex string, prefixed so we can detect already-hashed values
    std::ostringstream oss;
    oss << "H" << std::hex << hash;
    return oss.str();
}

bool data_isValidGrade(double value)
{
    return value >= MIN_GRADE && value <= MAX_GRADE;
}

int data_generateId(DataStore& store)
{
    return store.nextId++;
}
