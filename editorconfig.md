# EditorConfig 설정 가이드

이 문서는 프로젝트 루트에 있는 `.editorconfig` 파일의 각 설정 항목에 대한 설명을 담고 있습니다. EditorConfig는 다양한 에디터와 IDE 간에 일관된 코딩 스타일을 유지하도록 돕습니다.

## 전역 설정 (Root)
*   **root = true**
    *   이 파일이 프로젝트의 최상위 설정 파일임을 나타냅니다. 상위 디렉토리에서 다른 `.editorconfig` 파일을 찾지 않습니다.

## 모든 파일 기본 설정 (`[*]`)
*   **indent_style = tab**
    *   들여쓰기 방식으로 탭(tab)을 사용합니다.
*   **indent_size = 4**
    *   들여쓰기 너비를 4칸으로 설정합니다. (탭의 시각적 너비)
*   **charset = utf-8**
    *   파일 인코딩을 UTF-8로 설정합니다.
*   **end_of_line = crlf**
    *   줄바꿈 문자를 Windows 방식인 CRLF(`\r\n`)로 사용합니다.
*   **insert_final_newline = true**
    *   파일 저장 시 마지막 줄에 항상 빈 줄을 추가합니다.
*   **trim_trailing_whitespace = true**
    *   줄 끝에 있는 불필요한 공백을 자동으로 제거합니다.

## C++ 전용 설정 (`[*.{cpp,h,hpp,c,cc,cxx,hxx}]`)
이 섹션은 Visual Studio의 C++ 포맷팅 규칙을 따릅니다.

### 들여쓰기 (Indentation)
*   **cpp_indent_braces = false**
    *   중괄호 자체를 들여쓰지 않습니다.
*   **cpp_indent_multi_line_relative_to = innermost_parenthesis**
    *   여러 줄에 걸친 코드를 가장 안쪽 괄호를 기준으로 정렬합니다.
*   **cpp_indent_within_parentheses = align_to_parenthesis**
    *   괄호 안의 내용을 괄호 시작 위치에 맞춰 정렬합니다.
*   **cpp_indent_preserve_within_parentheses = true**
    *   괄호 안의 기존 들여쓰기를 유지하려고 시도합니다.
*   **cpp_indent_case_contents = true**
    *   `switch`문의 `case` 내부 콘텐츠를 들여씁니다.
*   **cpp_indent_case_labels = false**
    *   `case` 레이블 자체는 `switch` 문과 같은 수준에 둡니다.
*   **cpp_indent_access_specifiers = false**
    *   `public:`, `private:` 등의 접근 제어자를 클래스 수준에 맞춥니다.
*   **cpp_indent_namespace_contents = false**
    *   `namespace` 안의 내용을 들여쓰지 않습니다.
*   **cpp_indent_preserve_comments = true**
    *   주석의 들여쓰기를 유지합니다.

### 줄바꿈 (Newline Settings - Allman Style)
*   **cpp_new_line_before_open_brace_namespace/type/function/block = new_line**
    *   네임스페이스, 클래스, 함수, 블록의 여는 중괄호를 다음 줄에 배치합니다.
*   **cpp_new_line_before_open_brace_lambda = same_line**
    *   람다 식의 여는 중괄호는 같은 줄에 둡니다.
*   **cpp_new_line_before_catch/else = true**
    *   `catch`와 `else`를 새로운 줄에서 시작합니다.
*   **cpp_new_line_before_while_in_do_while = false**
    *   `do-while` 문의 `while`을 닫는 중괄호와 같은 줄에 둡니다.

### 공백 (Spacing)
*   **cpp_space_before_function_open_parenthesis = remove**
    *   함수 이름과 여는 괄호 사이의 공백을 제거합니다. (`func()`)
*   **cpp_space_after_keywords_in_control_flow_statements = true**
    *   `if`, `for`, `while` 등 제어문 키워드 뒤에 공백을 둡니다.
*   **cpp_space_before_block_open_brace = true**
    *   코드 블록(`{`) 앞에 공백을 둡니다.
*   **cpp_space_between_empty_braces = true**
    *   빈 중괄호 `{ }` 사이에 공백을 둡니다.
*   **cpp_space_before_initializer_list_open_brace = true**
    *   초기화 리스트의 여는 중괄호 앞에 공백을 둡니다.
*   **cpp_space_within_initializer_list_braces = true**
    *   초기화 리스트 중괄호 안에 공백을 둡니다. `{ 1, 2 }`

### 포인터/참조 정렬
*   **cpp_pointer_alignment = left**
    *   포인터 기호(`*`)를 타입 쪽에 붙입니다. (`int* ptr`)

## 마크다운 파일 설정 (`[*.md]`)
*   **indent_style = space**
    *   마크다운 파일은 공백(space)으로 들여쓰기합니다.
*   **indent_size = 2**
    *   들여쓰기 너비를 2칸으로 설정합니다.

## XML 및 빌드 파일 설정 (`[*.{xml,vcxproj,filters,slnx,props,targets}]`)
*   **indent_style = space**
    *   XML 및 Visual Studio 프로젝트 관련 파일은 공백으로 들여쓰기합니다.
*   **indent_size = 2**
    *   들여쓰기 너비를 2칸으로 설정합니다.
