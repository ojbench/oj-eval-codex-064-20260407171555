#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <sstream>
#include <string_view>

using namespace std;

struct Token {
  enum Kind { Str, Signed, Unsigned, Vector } kind;
  string s;
  vector<Token> elems;
};

static void skip_ws(const string& in, size_t& pos) { while (pos < in.size() && isspace((unsigned char)in[pos])) ++pos; }

static bool parse_bracket_list(const string& in, size_t& pos, Token& tok);

static bool parse_token(const string& in, size_t& pos, Token& tok) {
  skip_ws(in, pos);
  if (pos >= in.size()) return false;
  if (in[pos] == '[') {
    return parse_bracket_list(in, pos, tok);
  }
  size_t start = pos;
  while (pos < in.size() && !isspace((unsigned char)in[pos])) ++pos;
  string t = in.substr(start, pos - start);
  if (!t.empty()) {
    bool neg = (t[0] == '-');
    string digits = neg ? t.substr(1) : t;
    bool all_digits = !digits.empty();
    for (char c : digits) if (!isdigit((unsigned char)c)) { all_digits = false; break; }
    if (all_digits) {
      tok.kind = neg ? Token::Signed : Token::Unsigned;
      tok.s = std::move(t);
      return true;
    }
  }
  tok.kind = Token::Str; tok.s = std::move(t);
  return true;
}

static bool parse_bracket_list(const string& in, size_t& pos, Token& tok) {
  if (in[pos] != '[') return false;
  ++pos;
  tok.kind = Token::Vector; tok.elems.clear();
  while (true) {
    skip_ws(in, pos);
    if (pos >= in.size()) break;
    if (in[pos] == ']') { ++pos; break; }
    Token child;
    if (!parse_token(in, pos, child)) break;
    tok.elems.push_back(std::move(child));
    skip_ws(in, pos);
    if (pos < in.size() && in[pos] == ',') { ++pos; continue; }
    if (pos < in.size() && in[pos] == ']') { ++pos; break; }
  }
  return true;
}

static bool parse_format_and_args(const string& line, string& fmt, vector<Token>& args) {
  fmt = line;
  args.clear();
  return true;
}

static bool need_more_args(const string& fmt, size_t have, size_t& needed) {
  needed = 0;
  for (size_t i = 0; i + 1 < fmt.size(); ++i) {
    if (fmt[i] == '%' && fmt[i+1] != '%') ++needed;
  }
  return have < needed;
}

static void print_vector_default(ostream& os, const vector<Token>& elems) {
  bool all_signed = !elems.empty();
  bool all_unsigned = !elems.empty();
  for (auto &e : elems) {
    if (e.kind == Token::Signed) { all_unsigned = false; }
    else if (e.kind == Token::Unsigned) { all_signed = false; }
    else { all_signed = all_unsigned = false; }
  }
  os << '[';
  for (size_t i = 0; i < elems.size(); ++i) {
    if (i) os << ',';
    if (all_signed) os << stoll(elems[i].s);
    else if (all_unsigned) { unsigned long long v=0; stringstream ss(elems[i].s); ss>>v; os<<v; }
    else os << elems[i].s;
  }
  os << ']';
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  string fmt;
  if (!getline(cin, fmt)) return 0;
  vector<Token> args;
  string line;
  while (getline(cin, line)) {
    size_t pos=0; while (true) { Token t; size_t save=pos; if (!parse_token(line,pos,t)) break; if (pos==save) break; args.push_back(std::move(t)); }
  }
  string_view sv(fmt);
  ostream& os = cout;
  size_t idx=0;
  while (true) {
    size_t p = sv.find('%');
    if (p==string_view::npos) { os<<sv; break; }
    os.write(sv.data(), (streamsize)p);
    if (p+1 >= sv.size()) break;
    char c = sv[p+1];
    if (c=='%') { os<<'%'; sv.remove_prefix(p+2); continue; }
    Token t = (idx<args.size()? args[idx] : Token{Token::Str, string(), {}});
    ++idx;
    if (c=='s') {
      if (t.kind==Token::Vector) print_vector_default(os,t.elems);
      else os<<t.s;
    } else if (c=='d') {
      long long v=0; try { v = stoll(t.s);} catch(...) { v=0; } os<<v;
    } else if (c=='u') {
      unsigned long long v=0; stringstream ss(t.s); ss>>v; os<<v;
    } else if (c=='_') {
      if (t.kind==Token::Vector) print_vector_default(os,t.elems);
      else if (t.kind==Token::Signed) os<<stoll(t.s);
      else if (t.kind==Token::Unsigned) { unsigned long long v=0; stringstream ss2(t.s); ss2>>v; os<<v; }
      else os<<t.s;
    }
    sv.remove_prefix(p+2);
  }
  return 0;
}
