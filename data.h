#pragma once

#include <string>
#include <vector>

 // ─────────────────────────────────────────────
 //  Constants
 // ─────────────────────────────────────────────

const int    MAX_SUBJECTS = 10;
const int    MAX_STUDENTS = 500;
const double MIN_GRADE = 2.0;
const double MAX_GRADE = 6.0;
const std::string DATA_FILE = "students.dat";
const std::string USERS_FILE = "users.dat";

// ─────────────────────────────────────────────
//  Enumerations
// ─────────────────────────────────────────────

// Represents the academic year of a student
enum class Grade
{
    FIRST = 1,
    SECOND = 2,
    THIRD = 3,
    FOURTH = 4
};

// Represents the gender of a student
enum class Gender
{
    MALE = 0,
    FEMALE = 1,
    OTHER = 2
};

// Represents a subject (предмет)
enum class Subject
{
    MATHEMATICS = 0,
    BULGARIAN = 1,
    ENGLISH = 2,
    HISTORY = 3,
    GEOGRAPHY = 4,
    PHYSICS = 5,
    CHEMISTRY = 6,
    BIOLOGY = 7,
    INFORMATICS = 8,
    PHYSICAL_ED = 9
};

// ─────────────────────────────────────────────
//  Data Structures
// ─────────────────────────────────────────────

/*
 * SubjectGrade – holds the grade for a single subject.
 */
struct SubjectGrade
{
    Subject subject = Subject::MATHEMATICS;
    double  value = 0.0;
    std::string note = "";
};

/*
 * Student – the primary record stored in the system.
 */
struct Student
{
    int         id = 0;
    std::string firstName = "";
    std::string lastName = "";
    std::string egn = "";
    Gender      gender = Gender::MALE;
    Grade       yearGroup = Grade::SECOND;
    std::string classLabel = "";
    std::vector<SubjectGrade> grades;
    bool        isActive = true;
};

/*
 * ClassRecord – groups students by class.
 */
struct ClassRecord
{
    std::string label;               // e.g. "9А"
    std::vector<int> studentIds;     // IDs of students in this class
};

/*
 * DataStore – the in-memory database.
 */
struct DataStore
{
    std::vector<Student>     students;
    std::vector<ClassRecord> classes;
    int                      nextId = 1;
};

// ─────────────────────────────────────────────
//  Function Prototypes – CRUD
// ─────────────────────────────────────────────

// Initialise an empty DataStore
DataStore data_createStore();

// Add a student to the store; returns the assigned ID
int data_addStudent(DataStore& store, const Student& student);

// Find a student by ID; returns nullptr if not found
Student* data_findById(DataStore& store, int id);

// Find students by last name (case-insensitive, partial match)
std::vector<Student*> data_findByLastName(DataStore& store, const std::string& lastName);

// Find students by class label
std::vector<Student*> data_findByClass(DataStore& store, const std::string& classLabel);

// Update an existing student record; returns true on success
bool data_updateStudent(DataStore& store, const Student& updated);

// Soft-delete a student (sets isActive = false)
bool data_deleteStudent(DataStore& store, int id);

// Return all active students
std::vector<Student*> data_getAllActive(DataStore& store);

// ─────────────────────────────────────────────
//  Function Prototypes – Grades
// ─────────────────────────────────────────────

// Add or overwrite a subject grade for a student
bool data_setGrade(DataStore& store, int studentId,
    Subject subject, double value,
    const std::string& note = "");

// Get the grade for a specific subject; returns -1.0 if not found
double data_getGrade(const Student& student, Subject subject);

// Remove a grade entry for a student
bool data_removeGrade(DataStore& store, int studentId, Subject subject);

// ─────────────────────────────────────────────
//  Function Prototypes – Class Management
// ─────────────────────────────────────────────

// Register a new class; returns false if it already exists
bool data_addClass(DataStore& store, const std::string& label);

// Assign a student to a class (removes from previous class first)
bool data_assignToClass(DataStore& store, int studentId,
    const std::string& classLabel);

// Get a list of all class labels
std::vector<std::string> data_getAllClassLabels(const DataStore& store);

// ─────────────────────────────────────────────
//  Function Prototypes – Persistence
// ─────────────────────────────────────────────

// Save the entire store to disk (CSV-like format)
bool data_saveToFile(const DataStore& store, const std::string& filepath);

// Load the store from disk; returns false if file doesn't exist
bool data_loadFromFile(DataStore& store, const std::string& filepath);

// ─────────────────────────────────────────────
//  Function Prototypes – Helpers
// ─────────────────────────────────────────────

// Convert Subject enum → human-readable Bulgarian string
std::string data_subjectName(Subject s);

// Convert Subject enum → short code (e.g. "MAT")
std::string data_subjectCode(Subject s);

// Parse a subject from its short code; returns Subject::MATHEMATICS on failure
Subject data_subjectFromCode(const std::string& code);

// Convert Gender enum → string
std::string data_genderName(Gender g);

// Convert Grade enum → string like "9-ти клас"
std::string data_gradeName(Grade g);

// Return a list of all subjects
std::vector<Subject> data_allSubjects();

// Simple hash for EGN (so we never store raw national IDs)
std::string data_hashEgn(const std::string& egn);

// Validate that a grade value is within [2.0, 6.0]
bool data_isValidGrade(double value);

// Generate a simple unique ID from the store's counter
int data_generateId(DataStore& store);

// ─────────────────────────────────────────────
//  User / Authentication
// ─────────────────────────────────────────────

// Role defines what a logged-in user can do
enum class UserRole
{
    ADMIN = 0,   // Full access: add/edit/delete students and grades
    TEACHER = 1,   // Can view all, edit grades only
    VIEWER = 2    // Read-only access
};

/*
 * User – a login account stored in users.dat
 */
struct User
{
    std::string username = "";
    std::string passwordHash = "";
    std::string displayName = "";
    UserRole    role = UserRole::VIEWER;
};

/*
 * UserStore – holds all registered users in memory
 */
struct UserStore
{
    std::vector<User> users;
};

// Initialise a UserStore and populate with default accounts
UserStore data_createUserStore();

// Find a user by username; returns nullptr if not found
const User* data_findUser(const UserStore& store, const std::string& username);

// Simple password hash (same DJB2 as EGN hash)
std::string data_hashPassword(const std::string& password);

// Convert UserRole to readable string
std::string data_roleName(UserRole role);

// Save users to file
bool data_saveUsers(const UserStore& store, const std::string& filepath);

// Load users from file; returns false if not found
bool data_loadUsers(UserStore& store, const std::string& filepath);
