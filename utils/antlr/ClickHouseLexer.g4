lexer grammar ClickHouseLexer;

// NOTE: don't forget to add new keywords to the parser rule "keyword"!

// Keywords

ACCESS: A C C E S S;
ADD: A D D;
AFTER: A F T E R;
ALIAS: A L I A S;
ALL: A L L;
ALTER: A L T E R;
AND: A N D;
ANTI: A N T I;
ANY: A N Y;
ARRAY: A R R A Y;
AS: A S;
ASCENDING: A S C | A S C E N D I N G;
ASOF: A S O F;
AST: A S T;
ASYNC: A S Y N C;
ATTACH: A T T A C H;
BETWEEN: B E T W E E N;
BOTH: B O T H;
BY: B Y;
CACHES: C A C H E S;
CASE: C A S E;
CAST: C A S T;
CHECK: C H E C K;
CLEAR: C L E A R;
CLUSTER: C L U S T E R;
CLUSTERS: C L U S T E R S;
CODEC: C O D E C;
COLLATE: C O L L A T E;
COLUMN: C O L U M N;
COLUMNS: C O L U M N S;
COMMENT: C O M M E N T;
CONSTRAINT: C O N S T R A I N T;
CREATE: C R E A T E;
CROSS: C R O S S;
CUBE: C U B E;
CURRENT: C U R R E N T;
CURRENT_USER: C U R R E N T UNDERSCORE U S E R;
CHANGED: C H A N G E D;
DATABASE: D A T A B A S E;
DATABASES: D A T A B A S E S;
DATE: D A T E;
DAY: D A Y;
DEDUPLICATE: D E D U P L I C A T E;
DEFAULT: D E F A U L T;
DELAY: D E L A Y;
DELETE: D E L E T E;
DESC: D E S C;
DESCENDING: D E S C E N D I N G;
DESCRIBE: D E S C R I B E;
DETACH: D E T A C H;
DICTIONARIES: D I C T I O N A R I E S;
DICTIONARY: D I C T I O N A R Y;
DISK: D I S K;
DISTINCT: D I S T I N C T;
DISTRIBUTED: D I S T R I B U T E D;
DROP: D R O P;
ELSE: E L S E;
ENABLED: E N A B L E D;
END: E N D;
ENGINE: E N G I N E;
ENGINES: E N G I N E S;
ESTIMATE: E S T I M A T E;
EVENTS: E V E N T S;
EXCEPT: E X C E P T;
EXISTS: E X I S T S;
EXPLAIN: E X P L A I N;
EXPRESSION: E X P R E S S I O N;
EXTENDED: E X T E N D E D;
EXTRACT: E X T R A C T;
FETCHES: F E T C H E S;
FIELDS: F I E L D S;
FILESYSTEM: F I L E S Y S T E M;
FILL: F I L L;
FINAL: F I N A L;
FIRST: F I R S T;
FLUSH: F L U S H;
FOLLOWING: F O L L O W I N G;
FOR: F O R;
FORMAT: F O R M A T;
FREEZE: F R E E Z E;
FROM: F R O M;
FULL: F U L L;
FUNCTION: F U N C T I O N;
FUNCTIONS: F U N C T I O N S;
GLOBAL: G L O B A L;
GRANULARITY: G R A N U L A R I T Y;
GRANTS: G R A N T S;
GROUP: G R O U P;
GROUPING: G R O U P I N G;
HAVING: H A V I N G;
HIERARCHICAL: H I E R A R C H I C A L;
HOUR: H O U R;
ID: I D;
IF: I F;
ILIKE: I L I K E;
IMPLICIT: I M P L I C I T;
IN: I N;
INDEX: I N D E X;
INDEXES: I N D E X E S;
INDICES: I N D I C E S;
INF: I N F | I N F I N I T Y;
INJECTIVE: I N J E C T I V E;
INNER: I N N E R;
INSERT: I N S E R T;
INTERPOLATE: I N T E R P O L A T E;
INTERVAL: I N T E R V A L;
INTO: I N T O;
IS: I S;
IS_OBJECT_ID: I S UNDERSCORE O B J E C T UNDERSCORE I D;
JOIN: J O I N;
KEY: K E Y;
KEYS: K E Y S;
KILL: K I L L;
LAST: L A S T;
LAYOUT: L A Y O U T;
LEADING: L E A D I N G;
LEFT: L E F T;
LIFETIME: L I F E T I M E;
LIKE: L I K E;
LIMIT: L I M I T;
LIVE: L I V E;
LOCAL: L O C A L;
LOGS: L O G S;
MATERIALIZE: M A T E R I A L I Z E;
MATERIALIZED: M A T E R I A L I Z E D;
MAX: M A X;
MERGES: M E R G E S;
MICROSECOND: M I C R O S E C O N D;
MILLISECOND: M I L L I S E C O N D;
MIN: M I N;
MINUTE: M I N U T E;
MODIFY: M O D I F Y;
MONTH: M O N T H;
MOVE: M O V E;
MUTATION: M U T A T I O N;
NAN_SQL: N A N; // conflicts with macro NAN
NANOSECOND: N A N O S E C O N D;
NO: N O;
NOT: N O T;
NULL_SQL: N U L L; // conflicts with macro NULL
NULLS: N U L L S;
OFFSET: O F F S E T;
ON: O N;
OPTIMIZE: O P T I M I Z E;
OR: O R;
ORDER: O R D E R;
OUTER: O U T E R;
OUTFILE: O U T F I L E;
OVER: O V E R;
OVERRIDE: O V E R R I D E;
PARTITION: P A R T I T I O N;
PIPELINE: P I P E L I N E;
PLAN: P L A N;
POLICY: P O L I C Y;
POLICIES: P O L I C I E S;
POPULATE: P O P U L A T E;
PRECEDING: P R E C E D I N G;
PREWHERE: P R E W H E R E;
PRIMARY: P R I M A R Y;
PRIVILEGES: P R I V I L E G E S;
PROCESSLIST: P R O C E S S L I S T;
PROFILE: P R O F I L E;
PROFILES: P R O F I L E S;
PROJECTION: P R O J E C T I O N;
QUARTER: Q U A R T E R;
QUOTA: Q U O T A;
QUOTAS: Q U O T A S;
RANGE: R A N G E;
RECURSIVE: R E C U R S I V E;
RELOAD: R E L O A D;
REMOVE: R E M O V E;
RENAME: R E N A M E;
REPLACE: R E P L A C E;
REPLICA: R E P L I C A;
REPLICATED: R E P L I C A T E D;
RIGHT: R I G H T;
ROLE: R O L E;
ROLES: R O L E S;
ROLLUP: R O L L U P;
ROW: R O W;
ROWS: R O W S;
SAMPLE: S A M P L E;
SECOND: S E C O N D;
SELECT: S E L E C T;
SEMI: S E M I;
SENDS: S E N D S;
SET: S E T;
SETS: S E T S;
SETTING: S E T T I N G;
SETTINGS: S E T T I N G S;
SHOW: S H O W;
SOURCE: S O U R C E;
START: S T A R T;
STOP: S T O P;
SUBSTRING: S U B S T R I N G;
SYNC: S Y N C;
SYNTAX: S Y N T A X;
SYSTEM: S Y S T E M;
STEP: S T E P;
TABLE: T A B L E;
TABLES: T A B L E S;
TEMPORARY: T E M P O R A R Y;
TEST: T E S T;
THEN: T H E N;
TIES: T I E S;
TIMEOUT: T I M E O U T;
TIMESTAMP: T I M E S T A M P;
TO: T O;
TOP: T O P;
TOTALS: T O T A L S;
TRAILING: T R A I L I N G;
TREE: T R E E;
TRIM: T R I M;
TRUNCATE: T R U N C A T E;
TTL: T T L;
TYPE: T Y P E;
UNBOUNDED: U N B O U N D E D;
UNION: U N I O N;
UPDATE: U P D A T E;
USE: U S E;
USER: U S E R;
USERS: U S E R S;
USING: U S I N G;
UUID: U U I D;
VALUES: V A L U E S;
VIEW: V I E W;
VOLUME: V O L U M E;
WATCH: W A T C H;
WEEK: W E E K;
WHEN: W H E N;
WHERE: W H E R E;
WINDOW: W I N D O W;
WITH: W I T H;
YEAR: Y E A R | Y Y Y Y;

JSON_FALSE: F A L S E;
JSON_TRUE: T R U E;

// Tokens

HEXADECIMAL_STRING_LITERAL: X QUOTE_SINGLE HEX_DIGIT+ QUOTE_SINGLE;
BINARY_STRING_LITERAL: B QUOTE_SINGLE BINARY_DIGIT+ QUOTE_SINGLE;

IDENTIFIER
    : (LETTER | UNDERSCORE) (LETTER | UNDERSCORE | DEC_DIGIT)*
    | BACKQUOTE ( ~([\\`]) | (BACKSLASH .) | (BACKQUOTE BACKQUOTE) )* BACKQUOTE
    | QUOTE_DOUBLE ( ~([\\"]) | (BACKSLASH .) | (QUOTE_DOUBLE QUOTE_DOUBLE) )* QUOTE_DOUBLE
    ;
FLOATING_LITERAL
    : HEXADECIMAL_NUMERIC_LITERAL DOT HEX_DIGIT* (P | E) (PLUS | DASH)? DEC_DIGIT+
    | HEXADECIMAL_NUMERIC_LITERAL (P | E) (PLUS | DASH)? DEC_DIGIT+
    | DECIMAL_LITERAL DOT DEC_DIGIT* E (PLUS | DASH)? DEC_DIGIT+
    | DOT DECIMAL_LITERAL E (PLUS | DASH)? DEC_DIGIT+
    | DECIMAL_LITERAL E (PLUS | DASH)? DEC_DIGIT+
    ;
OCTAL_LITERAL: '0' OCT_DIGIT+;
DECIMAL_LITERAL: DEC_DIGIT+;
HEXADECIMAL_NUMERIC_LITERAL: '0' X HEX_DIGIT+;
BINARY_NUMERIC_LITERAL: '0' B BINARY_DIGIT+;

// It's important that quote-symbol is a single character.
STRING_LITERAL: QUOTE_SINGLE ( ~([\\']) | (BACKSLASH .) | (QUOTE_SINGLE QUOTE_SINGLE) )* QUOTE_SINGLE;

// Alphabet and allowed symbols

fragment A: [aA];
fragment B: [bB];
fragment C: [cC];
fragment D: [dD];
fragment E: [eE];
fragment F: [fF];
fragment G: [gG];
fragment H: [hH];
fragment I: [iI];
fragment J: [jJ];
fragment K: [kK];
fragment L: [lL];
fragment M: [mM];
fragment N: [nN];
fragment O: [oO];
fragment P: [pP];
fragment Q: [qQ];
fragment R: [rR];
fragment S: [sS];
fragment T: [tT];
fragment U: [uU];
fragment V: [vV];
fragment W: [wW];
fragment X: [xX];
fragment Y: [yY];
fragment Z: [zZ];

fragment LETTER: [a-zA-Z];
fragment OCT_DIGIT: [0-7];
fragment DEC_DIGIT: [0-9];
fragment HEX_DIGIT: [0-9a-fA-F];
fragment BINARY_DIGIT: [0-1];

ARROW: '->';
ASTERISK: '*';
BACKQUOTE: '`';
BACKSLASH: '\\';
COLON: ':';
COMMA: ',';
CONCAT: '||';
DASH: '-';
DOT: '.';
EQ_DOUBLE: '==';
EQ_SINGLE: '=';
GE: '>=';
GT: '>';
LBRACE: '{';
LBRACKET: '[';
LE: '<=';
LPAREN: '(';
LT: '<';
NOT_EQ: '!=' | '<>';
PERCENT: '%';
PLUS: '+';
QUERY: '?';
QUOTE_DOUBLE: '"';
QUOTE_SINGLE: '\'';
RBRACE: '}';
RBRACKET: ']';
RPAREN: ')';
SEMICOLON: ';';
SLASH: '/';
UNDERSCORE: '_';

// Comments and whitespace

MULTI_LINE_COMMENT: '/*' .*? '*/' -> skip;
SINGLE_LINE_COMMENT: '--' ~('\n'|'\r')* ('\n' | '\r' | EOF) -> skip;
WHITESPACE: [ \u000B\u000C\t\r\n] -> skip;  // '\n' can be part of multiline single query
