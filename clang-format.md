# Clang-Format 설정 가이드

이 문서는 프로젝트 루트에 있는 `.clang-format` 파일의 각 설정 항목에 대한 설명을 담고 있습니다. 이 설정은 코드 스타일의 일관성을 유지하기 위해 사용됩니다.

## 기본 설정
*   **Language: Cpp**
    *   포맷팅을 적용할 언어를 C++로 지정합니다.
*   **BasedOnStyle: Microsoft**
    *   Microsoft의 기본 코딩 스타일 가이드를 기반으로 설정합니다.

## 들여쓰기 (Indentation)
*   **IndentWidth: 4**
    *   들여쓰기 너비를 4칸으로 설정합니다.
*   **TabWidth: 4**
    *   탭 문자의 너비를 4칸으로 설정합니다.
*   **UseTab: Always**
    *   들여쓰기에 항상 탭 문자를 사용합니다. ( `.editorconfig`와 일치시킴)

## 중괄호 스타일 (Bracing Style)
*   **BreakBeforeBraces: Allman**
    *   모든 중괄호를 다음 줄에서 시작하는 Allman 스타일(BSD 스타일)을 사용합니다.
    *   예:
        ```cpp
        if (condition)
        {
            // body
        }
        ```

## 포인터 및 참조 정렬 (Pointer/Reference Alignment)
*   **PointerAlignment: Left**
    *   포인터 기호(`*`)를 타입 쪽에 붙입니다.
    *   예: `int* ptr;`
*   **DerivePointerAlignment: false**
    *   파일 내의 기존 스타일에서 포인터 정렬 방식을 추측하지 않고 설정된 값을 강제합니다.

## 열 제한 (Column Limit)
*   **ColumnLimit: 120**
    *   한 줄의 최대 길이를 120자로 제한합니다. 이 길이를 넘으면 자동으로 줄바꿈을 시도합니다.

## 세부 포맷팅 규칙 (Detailed Formatting Rules)
*   **AccessModifierOffset: -4**
    *   `public:`, `private:`와 같은 접근 제어자를 클래스 들여쓰기 기준으로 -4칸(왼쪽으로) 이동시킵니다. 결과적으로 클래스 선언문과 수직으로 맞게 됩니다.
*   **AlignAfterOpenBracket: Align**
    *   여는 괄호(`(`, `[`, `{`) 뒤에 줄바꿈이 일어날 경우, 다음 줄의 인자들을 괄호 시작 위치에 맞게 정렬합니다.
*   **AlignConsecutiveAssignments: false**
    *   연속된 줄의 대입 연산자(`=`)를 세로로 맞추지 않습니다.
*   **AlignConsecutiveDeclarations: false**
    *   연속된 줄의 변수 선언 이름을 세로로 맞추지 않습니다.
*   **AlignEscapedNewlines: Right**
    *   매크로 등에서 사용되는 이스케이프된 개행(`\`)을 최대한 오른쪽에 맞춥니다.
*   **AlignOperands: true**
    *   줄바꿈된 연산자들을 서로 맞춥니다.
*   **AlignTrailingComments: true**
    *   코드 뒤에 오는 주석들을 세로로 맞춥니다.
*   **AllowAllParametersOfDeclarationOnNextLine: true**
    *   함수 선언 시 모든 매개변수를 다음 줄로 넘기는 것을 허용합니다.
*   **AllowShortBlocksOnASingleLine: false**
    *   짧은 블록(`{ ... }`)을 한 줄에 쓰는 것을 허용하지 않습니다.
*   **AllowShortCaseLabelsOnASingleLine: false**
    *   `case` 문을 한 줄에 쓰는 것을 허용하지 않습니다.
*   **AllowShortFunctionsOnASingleLine: None**
    *   함수 본문을 한 줄에 쓰는 것을 어떤 경우에도 허용하지 않습니다.
*   **AllowShortIfStatementsOnASingleLine: false**
    *   `if` 문을 한 줄에 쓰는 것을 허용하지 않습니다.
*   **AllowShortLoopsOnASingleLine: false**
    *   `for`, `while` 등의 반복문을 한 줄에 쓰는 것을 허용하지 않습니다.

## 인클루드 관리 (Include Management)
*   **SortIncludes: true**
    *   `#include` 문을 알파벳 순서로 정렬합니다.
*   **IncludeBlocks: Regroup**
    *   여러 인클루드 블록을 규칙에 따라 다시 그룹화하고 정렬합니다.

## 언어 표준 및 공백 설정
*   **Standard: Latest**
    *   최신 C++ 표준을 기준으로 포맷팅합니다.
*   **SpaceAfterCStyleCast: false**
    *   C 스타일 형변환(`(int)x`) 뒤에 공백을 두지 않습니다.
*   **SpaceBeforeAssignmentOperators: true**
    *   대입 연산자(`=`, `+=` 등) 앞에 공백을 둡니다.
*   **SpaceBeforeParens: ControlStatements**
    *   제어문(`if`, `for`, `while` 등)의 괄호 앞에만 공백을 둡니다.
*   **SpaceInEmptyParentheses: false**
    *   빈 괄호 `()` 안에 공백을 두지 않습니다.
*   **SpacesBeforeTrailingComments: 1**
    *   코드와 뒤따르는 주석 사이에 최소 1칸의 공백을 둡니다.
*   **SpacesInCStyleCastParentheses: false**
    *   C 스타일 형변환 괄호 안에 공백을 두지 않습니다.
*   **SpacesInContainerLiterals: false**
    *   컨테이너 리터럴 안에 공백을 두지 않습니다.
*   **SpacesInParentheses: false**
    *   괄호 `()` 안에 공백을 두지 않습니다.
*   **SpacesInSquareBrackets: false**
    *   대괄호 `[]` 안에 공백을 두지 않습니다.
