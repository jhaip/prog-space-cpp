#include <sol/sol.hpp>

class SourceCodeManager {
  private:
    sol::state lua;
    std::vector<std::string> scriptPaths;
    std::vector<std::string> scriptsSourceCodes;
    std::vector<int> prevSeenPrograms;

  public:
    SourceCodeManager(std::vector<std::string> _scriptPaths) : scriptPaths(_scriptPaths), scriptsSourceCodes(scriptPaths.size()) {}

    void init(Database &db) {
        int scriptPathIndex = 0;
        for (const auto &scriptPath : scriptPaths) {
            if (scriptPath.length() > 0) {
                auto sourceCode = read_file(scriptPath);
                scriptsSourceCodes[scriptPathIndex] = sourceCode;
                db.claim(Fact{{Term{"#00"}, Term{std::to_string(scriptPathIndex)}, Term{"source"}, Term{"code"}, Term{"", sourceCode}}});
            }
            scriptPathIndex++;
        }

        init_lua_state(db);
    }
    void run_program(int id) {
        if (id < scriptsSourceCodes.size()) {
            std::cout << "running " << id << std::endl;
            try {
                auto result = lua.safe_script(scriptsSourceCodes[id], sol::script_pass_on_error);
                if (!result.valid()) {
                    sol::error err = result;
                    std::cerr << "The code has failed to run!\n"
                              << err.what() << "\nPanicking and exiting..."
                              << std::endl;
                }
            } catch (const std::exception &e) {
                std::cout << "Exception when running program " << id << ": " << e.what() << std::endl;
            }
        }
    }
    void stop_program(Database &db, int id) {
        std::cout << id << " died." << std::endl;
        if (id < scriptsSourceCodes.size()) {
            db.cleanup(std::to_string(id));
        }
    }
    void update(Database &db) {
        auto seenProgramResults = db.select({"$ program $id at $ $ $ $ $ $ $ $"});
        std::vector<int> seenPrograms(seenProgramResults.size());
        std::transform(seenProgramResults.begin(), seenProgramResults.end(), seenPrograms.begin(),
            [](auto result){ return std::stoi(result.get("id").value); });
        
        std::vector<int> newlySeenPrograms;
        std::vector<int> programsThatDied;
        for (auto &id : seenPrograms) {
            if (std::find(prevSeenPrograms.begin(), prevSeenPrograms.end(), id) == prevSeenPrograms.end()) {
                newlySeenPrograms.emplace_back(id);
            }
        }
        for (auto &id : prevSeenPrograms) {
            if (std::find(seenPrograms.begin(), seenPrograms.end(), id) == seenPrograms.end()) {
                programsThatDied.emplace_back(id);
            }
        }
        prevSeenPrograms = seenPrograms;

        for (auto &id : programsThatDied) {
            stop_program(db, id);
        }
        for (auto &id : newlySeenPrograms) {
            run_program(id);
        }

        auto results = db.select({"$ wish $programId source code is $code"});
        if (results.size() > 0) {
            for (const auto &result : results) {
                Term programIdTerm{"", ""};
                Term sourceCodeTerm{"", ""};
                for (const auto &resultVariable : result.Result) {
                    if (resultVariable.first == "programId") {
                        programIdTerm = resultVariable.second;
                    } else if (resultVariable.first == "code") {
                        sourceCodeTerm = resultVariable.second;
                    }
                }
                int programId = stoi(programIdTerm.value);
                db.retract("$ " + programIdTerm.value + " source code $");
                db.claim(Fact{{Term{"#00"}, programIdTerm, Term{"source"}, Term{"code"}, sourceCodeTerm}});
                db.cleanup(programIdTerm.value);
                scriptsSourceCodes[programId] = sourceCodeTerm.value;
                auto programIsRunningResults = db.select({"$ program " + programIdTerm.value + " at $ $ $ $ $ $ $ $"});
                if (programIsRunningResults.size() > 0) {
                    run_program(programId);
                }
                write_to_file(scriptPaths[programId], sourceCodeTerm.value);
            }
            db.retract("$ wish $ source code is $");
        }
    }

  private:
    void init_lua_state(Database &db) {
        // open those basic lua libraries
        // again, for print() and other basic utilities
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::os);

        Illumination::add_illumination_usertype_to_lua(lua);

        lua.set_function("claim", [&db](sol::variadic_args va) {
            std::vector<Term> terms;
            for (auto v : va) {
                sol::optional<sol::table> table = v;
                if (table != sol::nullopt) {
                    auto tableVal = table.value();
                    terms.push_back(Term{tableVal[1], tableVal[2]});
                } else {
                    std::string s = v;
                    auto subfact = Fact{s};
                    for (const auto &t : subfact.terms) {
                        terms.push_back(t);
                    }
                }
            }
            db.claim(Fact{terms});
        });
        lua.set_function("cleanup", &Database::cleanup, &db);
        lua.set_function("retract", &Database::retract, &db);
        lua.set_function("when", &Database::when, &db);
        lua.set_function("remove_subs", &Database::remove_subs, &db);
        lua.set_function("http_request", http_request);
    }
    std::string read_file(std::string filepath) {
        std::ifstream t(filepath);
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }
    void write_to_file(std::string filepath, std::string contents) {
        std::ofstream out(filepath, std::ofstream::trunc); // override file contents
        out << contents;
        out.close();
    }
};