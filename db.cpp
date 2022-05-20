#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <iostream>
#include <map>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
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
        // } else if (input.size() > 1 && input[0] == '"' && input[input.size()-1] == '"') { // this may be difference syntax than prog-space
        //     value = input.substr(1, input.size()-2);
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
    // std::unordered_map<std::string, Term> Result;
    std::vector<std::pair<std::string, Term>> Result;

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

    std::optional<Term> find(const std::string key) const {
        for (const auto &n : Result)
        {
            if (n.first == key) {
                return n.second;
            }
        }
        return {};
    }

    Term get(const std::string key) const {
        return find(key).value();
    }

    void add(const std::string variable_name, const Term &term)
    {
        // Result.emplace(variable_name, term);
        Result.push_back({variable_name, term});
    }
};

std::vector<std::string> mysplit(const std::string &input)
{
    std::istringstream iss(input);
    std::vector<std::string> v;
    std::string s;

    while (iss >> std::quoted(s))
    {
        v.push_back(s);
    }
    return v;
}

class Fact {
public:
    std::vector<Term> terms;

    Fact(const std::vector<Term> _terms) : terms(_terms) {}

    Fact(const std::string input) {
        // parse fact as a string into a list of typed Terms
        std::vector<std::string> termStrs = mysplit(input);
        for (const auto &s : termStrs)
        {
            terms.push_back(Term{s});
        }
    }

    std::string toString() const {
        return terms_to_string(terms);
    }
};

bool term_match(const Term &A, const Term &B, QueryResult &env) {
    if (A.type == "variable" || A.type == "postfix") {
        auto variable_name = A.value;
        // "Wilcard" matches all but doesn't have a result
        if (variable_name == "") {
            return true;
        }
        if (auto match = env.find(variable_name)) {
            return term_match(match.value(), B, env);
        } else {
            env.add(variable_name, B);
            return true;
        }
    } else if (A.type == B.type && A.value == B.value) {
        return true;
    }
    return false;
}

bool fact_match(const Fact &A, const Fact &B, QueryResult &env) {
    if (A.terms.at(A.terms.size()-1).type == "postfix") {
        if (A.terms.size() > B.terms.size()) {
            return false;
        }
    } else if (A.terms.size() != B.terms.size()) {
        return false;
    }
    int i = 0;
    for (auto &A_term: A.terms) {
        auto did_match = term_match(A_term, B.terms[i], env);
        if (did_match == false) {
            return false;
        }
        if (A_term.type == "postfix") {
            auto postfix_variable_name = A_term.value;
            if (postfix_variable_name != "") {
                std::vector<Term> sliced_terms = std::vector<Term>(B.terms.begin() + i, B.terms.end()); // B.terms[i:]
                env.add(postfix_variable_name, Term{"text", terms_to_string(sliced_terms)});
            }
            break;
        }
        i++;
    }
    return true;
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
    std::vector<Fact> facts;
    std::vector<Subscription> subscriptions;
    bool debug = false;

    void print() {
        std::cout << "DATABASE:" << std::endl;
        for (const auto& f : facts) {
            std::cout << f.toString() << std::endl;
        }
        std::cout << "SUBS:" << std::endl;
        for (const auto &s : subscriptions)
        {
            std::cout << "sub from: " << s.program_source_id << std::endl;
        }
    }

    void claim(const Fact &fact) {
        if (debug)
            std::cout << "Claim:" << fact.toString() << std::endl;
        facts.push_back(fact);
    }

    std::vector<QueryResult> collect_solutions(const std::vector<Fact> &query, const QueryResult &env)
    {
        // std::cout << "collect solutions" << std::endl;
        if (query.size() == 0)
        {
            return {env};
        }
        std::vector<QueryResult> solutions{};
        for (const auto &f : facts)
        {
            QueryResult new_env{env};
            if (fact_match(query[0], f, new_env))
            {
                std::vector<Fact> sliced_query = std::vector<Fact>(query.begin() + 1, query.end()); // query[1:]
                auto collected_solutions = collect_solutions(sliced_query, new_env);
                for (const auto &solution : collected_solutions)
                {
                    solutions.push_back(solution);
                }
            }
        }
        return solutions;
    }

    std::vector<QueryResult> select(std::vector<std::string> query_parts) {
        std::vector<Fact> query;
        for (const auto &query_str : query_parts) {
            if (debug) std::cout << query_str << std::endl;
            query.push_back(Fact{query_str});
        }
        auto r = collect_solutions(query, QueryResult{});
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
        for (auto &sub : subscriptions) {
            auto results = select(sub.query_parts);
            if (sub.last_results != results) {
                // std::cout << "RESULTS are different!:" << std::endl;
                sub.last_results = results;
                if (debug)
                    std::cout << "RESULTS:" << std::endl;
                std::vector<std::map<std::string, std::string>> luaResults;
                for (const auto &result : results) 
                {
                    if (debug)
                        std::cout << "    " << result.toString() << std::endl;
                    luaResults.emplace_back(result.toLuaType());
                }
                try {
                    sub.callback_func(luaResults);
                }
                catch (const std::exception &e)
                {
                    std::cout << "Exception when running callback for program " << sub.program_source_id << ": " << e.what() << std::endl;
                }
            }
        }
    }

    void retract(std::string factQueryStr)
    {
        Fact factQuery{factQueryStr};
        auto it = facts.begin();
        while (it != facts.end())
        {
            QueryResult qr;
            if (fact_match(factQuery, *it, qr))
            {
                it = facts.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    void cleanup(std::string id) {
        auto it = facts.begin();
        while (it != facts.end())
        {
            if (!(*it).terms.empty() && (*it).terms[0].value == id)
            {
                it = facts.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    void remove_subs(std::string id) {
        auto it = subscriptions.begin();
        while (it != subscriptions.end())
        {
            if ((*it).program_source_id == id)
            {
                it = subscriptions.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
};
