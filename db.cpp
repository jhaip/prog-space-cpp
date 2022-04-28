#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>
#include <map>
#include <vector>
#include <chrono>
using namespace std::chrono;

struct Term {
    std::string type;
    std::string value;

    Term(std::string t, std::string v) : type(t), value(v) {}
    Term(const std::string input) {
        if (input[0] == '$') {
            type = "variable";
            value = input.substr(1);
        } else if (input[0] == '%') {
            type = "postfix";
            value = input.substr(1);
        } else if (input[0] == '#') { // this may be difference syntax than prog-space
            type = "id";
            value = input.substr(1);
        } else {
            value = input;
        }
    }

    bool operator==(const Term &other) const
    {
        return type == other.type && value == other.value;
    }

    std::string toString() const {
        if (type == "variable") {
            return "$" + value;
        } else if (type == "postfix") {
            return "%" + value;
        } else if (type == "id") {
            return "#" + value;
        }
        return value;
    }
};

std::string terms_to_string(std::vector<Term> terms) {
    std::string s = "";
    for (const auto &part : terms) {
        if (s.length() > 0) {
            s += " ";
        }
        s += part.toString();
    }
    return s;
}

struct QueryResult {
    std::map<std::string, Term> Result;

    bool operator==(const QueryResult &other) const
    {
        return Result == other.Result;
    }

    std::string toString() const {
        std::string s;
        for (const auto& n : Result) {
            s += n.first + ": " + n.second.toString() + ", ";
        }
        return s;
    }

    std::map<std::string, std::string> toLuaType() const {
        std::map<std::string, std::string> lua_result{};
        for (const auto& n : Result) {
            // TODO: convert to more specific types than string (int, etc.)
            lua_result.emplace(n.first, n.second.toString());
        }
        return lua_result;
    }
};

class Fact {
public:
    std::vector<Term> terms;

    Fact(const std::string input) {
        // parse fact as a string into a list of typed Terms
        int start = 0;
        int end = input.find(" ");
        while (end != -1) {
            terms.push_back(Term{input.substr(start, end - start)});
            start = end + 1;
            end = input.find(" ", start);
        }
        terms.push_back(Term{input.substr(start, end - start)});
    }

    std::string toString() const {
        return terms_to_string(terms);
    }

    bool fact_has_variables_or_wildcards() {
        for (const auto &t : terms)
        {
            if (t.type == "variable" || t.type == "postfix") {
                return true;
            }
        }
        return false;
    }
};

std::tuple<bool, QueryResult> term_match(const Term &A, const Term &B, const QueryResult &env) {
    if (A.type == "variable" || A.type == "postfix") {
        auto variable_name = A.value;
        // "Wilcard" matches all but doesn't have a result
        if (variable_name == "") {
            return {true, env};
        }
        if (env.Result.find(variable_name) != env.Result.end()) {
            return term_match(env.Result.at(variable_name), B, env);
        } else {
            QueryResult new_env{};
            for (const auto& n : env.Result) {
                new_env.Result.emplace(n.first, n.second);
            }
            new_env.Result.emplace(variable_name, B);
            return {true, new_env};
        }
    } else if (A.type == B.type && A.value == B.value) {
        return {true, env};
    }
    return {false, QueryResult{}};
}

std::tuple<bool, QueryResult> fact_match(const Fact &A, const Fact &B, const QueryResult &env) {
    if (A.terms.at(A.terms.size()-1).type == "postfix") {
        if (A.terms.size() > B.terms.size()) {
            return {false, QueryResult{}};
        }
    } else if (A.terms.size() != B.terms.size()) {
        return {false, QueryResult{}};
    }
    QueryResult new_env{};
    for (const auto& n : env.Result) {
        new_env.Result.emplace(n.first, n.second);
    }
    int i = 0;
    for (auto &A_term: A.terms) {
        auto [did_match, tmp_new_env] = term_match(A_term, B.terms[i], new_env);
        if (did_match == false) {
            return {false, QueryResult{}};
        }
        new_env = tmp_new_env;
        if (A_term.type == "postfix") {
            auto postfix_variable_name = A_term.value;
            if (postfix_variable_name != "") {
                std::vector<Term> sliced_terms = std::vector<Term>(B.terms.begin() + i, B.terms.end()); // B.terms[i:]
                new_env.Result.emplace(postfix_variable_name, Term{"text", terms_to_string(sliced_terms)});
            }
            break;
        }
        i++;
    }
    return {true, new_env};
}

std::vector<QueryResult> collect_solutions(const std::map<std::string, Fact> &facts, const std::vector<Fact> &query, const QueryResult &env) {
    auto start = high_resolution_clock::now();
    // std::cout << "collect solutions" << std::endl;
    if (query.size() == 0) {
        return {env};
    }
    std::vector<QueryResult> solutions{};
    std::vector<std::string> keys;
    for (const auto& n : facts) {
        keys.push_back(n.first);
    }
    std::sort(keys.begin(), keys.end());
    // std::cout << "done sorting " << (duration_cast<microseconds>(high_resolution_clock::now() - start)).count() << std::endl;
    for (const auto& k : keys) {
        auto [did_match, new_env] = fact_match(query[0], facts.at(k), env);
        if (did_match) {
            std::vector<Fact> sliced_query = std::vector<Fact>(query.begin() + 1, query.end()); // query[1:]
            auto collected_solutions = collect_solutions(facts, sliced_query, new_env);
            for (const auto& solution : collected_solutions) {
                solutions.push_back(solution);
            }
        }
    }
    // std::cout << "done looking for matches " << (duration_cast<microseconds>(high_resolution_clock::now() - start)).count() << std::endl;
    return solutions;
}

class Subscription
{
public:
    std::string program_source_id;
    std::vector<std::string> query_parts;
    // TODO: callback_func environment;
    sol::protected_function callback_func;
    std::vector<QueryResult> last_results;

    Subscription(std::string s, std::vector<std::string> qp, sol::protected_function c) : program_source_id(s), query_parts(qp), callback_func(c) {}
};

class Database
{
public:
    std::map<std::string, Fact> facts;
    std::vector<Subscription> subscriptions;
    bool debug = false;

    void print() {
        std::cout << "DATABASE:" << std::endl;
        for (const auto& n : facts) {
            std::cout << n.second.toString() << std::endl;
        }
        std::cout << "SUBS:" << std::endl;
        for (const auto &s : subscriptions)
        {
            std::cout << "sub from: " << s.program_source_id << std::endl;
        }
    }

    void claim(std::string fact_string) {
        if (debug) std::cout << "Claim:" << fact_string << std::endl;
        facts.emplace(fact_string, Fact{fact_string});
    }

    std::vector<QueryResult> select(std::vector<std::string> query_parts) {
        auto start = high_resolution_clock::now();
        std::vector<Fact> query;
        for (const auto &query_str : query_parts) {
            if (debug) std::cout << query_str << std::endl;
            query.push_back(Fact{query_str});
        }
        std::cout << "make query: " << (duration_cast<microseconds>(high_resolution_clock::now() - start)).count() << std::endl;
        auto r = collect_solutions(facts, query, QueryResult{});
        std::cout << "get solutions: " << (duration_cast<microseconds>(high_resolution_clock::now() - start)).count() << std::endl;
        return r;
    }

    void when(std::vector<std::string> query_parts, sol::protected_function callback_func) {
        if (debug) std::cout << "When" << std::endl;
        auto results = select(query_parts);
        if (debug) std::cout << "RESULTS:" << std::endl;
        for (const auto &result : results) {
            if (debug) std::cout << "    " << result.toString() << std::endl;
            callback_func(result.toLuaType());
        }
    }

    void register_when(std::string source, std::vector<std::string> query_parts, sol::protected_function callback_func) {
        subscriptions.push_back(Subscription{source, query_parts, callback_func});
    }

    void run_subscriptions() {
        auto start = high_resolution_clock::now();
        for (auto &sub : subscriptions) {
            auto results = select(sub.query_parts);
            std::cout << "1 " << (duration_cast<microseconds>(high_resolution_clock::now() - start)).count() << std::endl;
            if (sub.last_results != results) {
                // std::cout << "RESULTS are different!:" << std::endl;
                sub.last_results = results;
                std::cout << "2 " << (duration_cast<microseconds>(high_resolution_clock::now() - start)).count() << std::endl;
                if (debug)
                    std::cout << "RESULTS:" << std::endl;
                for (const auto &result : results) 
                {
                    if (debug)
                        std::cout << "    " << result.toString() << std::endl;
                    sub.callback_func(result.toLuaType());
                    
                }
                std::cout << "3 " << (duration_cast<microseconds>(high_resolution_clock::now() - start)).count() << std::endl;
            }
        }
    }

    void retract(std::string factQueryStr)
    {
        Fact factQuery{factQueryStr};
        if (factQuery.fact_has_variables_or_wildcards())
        {
            std::vector<std::string> keysToDelete;
            for (const auto& f : facts) {
                auto [did_match, _] = fact_match(factQuery, f.second, QueryResult{});
                if (did_match) {
                    keysToDelete.push_back(f.first);
                }
            }
            for (const auto& factKey : keysToDelete) {
                facts.erase(factKey);
            }
        }
        else
        {
            facts.erase(factQuery.toString());
        }
    }

    void cleanup(std::string id) {
        std::vector<std::string> keysToDelete;
        for (const auto& n : facts) {
            if (n.second.terms[0].value == id) {
                keysToDelete.push_back(n.first);
            }
        }
        for (const auto& factKey : keysToDelete) {
            facts.erase(factKey);
        }
    }
};
