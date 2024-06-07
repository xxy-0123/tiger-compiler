%filenames = "scanner"

 /*
  * Please don't modify the lines above.
  */

 /* You can add lex definitions here. */
digit [0-9]
letter [a-zA-Z]

%x COMMENT STR IGNORE

%%

 /*
  * Below is examples, which you can wipe out
  * and write regular expressions and actions of your own.
  *
  * All the tokens:
  *   Parser::ID
  *   Parser::STRING
  *   Parser::INT
  *   Parser::COMMA
  *   Parser::COLON
  *   Parser::SEMICOLON
  *   Parser::LPAREN
  *   Parser::RPAREN
  *   Parser::LBRACK
  *   Parser::RBRACK
  *   Parser::LBRACE
  *   Parser::RBRACE
  *   Parser::DOT
  *   Parser::PLUS
  *   Parser::MINUS
  *   Parser::TIMES
  *   Parser::DIVIDE
  *   Parser::EQ
  *   Parser::NEQ
  *   Parser::LT
  *   Parser::LE
  *   Parser::GT
  *   Parser::GE
  *   Parser::AND
  *   Parser::OR
  *   Parser::ASSIGN
  *   Parser::ARRAY
  *   Parser::IF
  *   Parser::THEN
  *   Parser::ELSE
  *   Parser::WHILE
  *   Parser::FOR
  *   Parser::TO
  *   Parser::DO
  *   Parser::LET
  *   Parser::IN
  *   Parser::END
  *   Parser::OF
  *   Parser::BREAK
  *   Parser::NIL
  *   Parser::FUNCTION
  *   Parser::VAR
  *   Parser::TYPE
  */

 /* reserved words */
"array" {adjust(); return Parser::ARRAY;}


 /* TODO: Put your lab2 code here */
"," {adjust(); return Parser::COMMA;}
":" {adjust(); return Parser::COLON;}
";" {adjust(); return Parser::SEMICOLON;}
"(" {adjust(); return Parser::LPAREN;}
")" {adjust(); return Parser::RPAREN;}
"[" {adjust(); return Parser::LBRACK;}
"]" {adjust(); return Parser::RBRACK;}
"{" {adjust(); return Parser::LBRACE;}
"}" {adjust(); return Parser::RBRACE;}
"." {adjust(); return Parser::DOT;}
"+" {adjust(); return Parser::PLUS;}
"-" {adjust(); return Parser::MINUS;}
"*" {adjust(); return Parser::TIMES;}
"/" {adjust(); return Parser::DIVIDE;}
"=" {adjust(); return Parser::EQ;}
"<>" {adjust(); return Parser::NEQ;}
"<" {adjust(); return Parser::LT;}
"<=" {adjust(); return Parser::LE;}
">" {adjust(); return Parser::GT;}
">=" {adjust(); return Parser::GE;}
"&" {adjust(); return Parser::AND;}
"|" {adjust(); return Parser::OR;}
":=" {adjust(); return Parser::ASSIGN;}
"if" {adjust(); return Parser::IF;}
"then" {adjust(); return Parser::THEN;}
"else" {adjust(); return Parser::ELSE;}
"for" {adjust(); return Parser::FOR;}
"while" {adjust(); return Parser::WHILE;}
"to" {adjust(); return Parser::TO;}
"do" {adjust(); return Parser::DO;}
"let" {adjust(); return Parser::LET;}
"in" {adjust(); return Parser::IN;}
"end" {adjust(); return Parser::END;}
"of" {adjust(); return Parser::OF;}
"break" {adjust(); return Parser::BREAK;}
"nil" {adjust(); return Parser::NIL;}
"function" {adjust(); return Parser::FUNCTION;}
"var" {adjust(); return Parser::VAR;}
"type" {adjust(); return Parser::TYPE;}

[a-zA-Z][_a-zA-Z0-9]* {adjust(); string_buf_ = matched(); return Parser::ID;}
{digit}+ {adjust(); return Parser::INT;}

"/*"             {
  adjust();
  comment_level_ = 1;
  begin(StartCondition__::COMMENT);
}
<COMMENT>
"/*"      {
  adjust();
  comment_level_++;
}
<COMMENT>
"*/"      {
  adjust();
  if(comment_level_ >= 2){comment_level_--;}
  else{begin(StartCondition__::INITIAL);}
}
<COMMENT>
[a]      {
  adjust();
}
<COMMENT>
[^a]     {
  adjust();
}

"\""            { // 开始字符串
  adjust();
  string_buf_="";
  begin(StartCondition__::STR);
}
<STR>"\""       { // 结束字符串
  adjustStr();
  setMatched(string_buf_);
  begin(StartCondition__::INITIAL);
  return Parser::STRING;
}
<STR>[[:alnum:]]      {
  adjustStr(); 
  string_buf_+=matched(); // 将转义字符转换为实际字符加入字符串缓冲区
}
<STR>     {
  [^[:alnum:]]  {adjustStr(); string_buf_+=matched(); }
  \\n   {adjustStr(); string_buf_ += "\n";} //\n
  \\t   {adjustStr(); string_buf_ += "\t";} //\t
  \\^[A-Z]  {adjustStr(); string_buf_ += (matched()[2] - 'A' + 1);}  //\^c
  \\\"  {adjustStr(); string_buf_ += "\"";} // "
  \\[0-9]{3}  {adjustStr(); 
    int value = 0;
    for (int i = 1; i <= 3; i++) {
        value = value * 10 + (matched()[i] - '0');
    }
    string_buf_ += value;} //\ddd
  \\[\n\t\f ]+\\   {adjustStr();} //\f__f\(start with space\tab\newline)
  "\\\\"  {adjustStr(); string_buf_ += "\\";}
}



 /*
  * skip white space chars.
  * space, tabs and LF
  */
[ \t]+ {adjust();}
\n {adjust(); errormsg_->Newline();}
\r\n {adjust(); errormsg_->Newline();}

 /* illegal input */
. {adjust(); errormsg_->Error(errormsg_->tok_pos_, "illegal token");}