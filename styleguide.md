# C++ Style Guide for SLAM

## Header Files

### 1. Self-contained Headers

> **標頭檔應該能夠獨自被編譯（self-contained），以 `.h` 結尾。至於用來插入其他檔案的文件，說到底它們並不是標頭檔，所以應以 `.inc` 結尾。**

所有標頭檔要能夠 self-contained。換言之，使用者和重構工具不需要為了使用一個標頭檔而引入額外更多的標頭檔。特別是，一個標頭檔應該要有 [header guards](#2-the-define-guard)。

如果 `.h` 文件宣告了一個 templates 或內聯 (inline) 函式，那他的 declaration 與 definition 必須在同一個 `.h`中，否則程式可能會在構建中連結失敗。有個例外：如果某函式樣板為所有相關模板參數顯式實例化，或本身就是類別的一個私有成員，那麼它就只能定義在實例化該模板的 `.cc` 文件裡。

只有少數的例外，一個標頭檔不是自我滿足的而是用來安插到程式碼某處裡。例如某些文件會被重複的 include 或是文件內容實際上是特定平台（platform-specific）擴展部分。這些文件就要用 .inc 文件擴展名。

### 2. The #define Guard

> **所有標頭檔都應該使用 `#define` 防止標頭檔被多次引入。建議的命名格式為 `<PROJECT>_<PATH>_<FILE>_H_`**

為保證唯一性，標頭檔的命名應該依據所在專案的完整路徑。例如：專案 foo 中的標頭檔 foo/src/bar/baz.h 可按如下方式保護：

```cpp
#ifndef FOO_BAR_BAZ_H_
#define FOO_BAR_BAZ_H_
…
#endif // FOO_BAR_BAZ_H_
```

### 3. Include What You Use

> **如果標頭檔參考了在其他地方定義的 symbol，那必須直接 include 該 header file，否則不應該做多餘的 include。任何情況都不應依賴過渡引入(transitive inclusions)。**

顧名思義，只應引入有用到的 symbol。

並且，我們不推薦使用過渡引入。這使得我們可以很輕易的刪除不再使用的 header file 並且不用擔心會導致 clients 編譯失敗。反之，這代表當 `foo.cc` 需要使用 `bar.h` 定義的 symbol 時必須直接引入，儘管 `foo.h` 已經引入過。

### 4. Forward Declarations

> **避免使用前置宣告。反之，[直接引入需要的標頭檔即可](#3-include-what-you-use)。**

**Definition:**

前置宣告是不提供與之關連的定義下，宣告一個類別、函式或是樣板。

```cpp
// In a C++ source file:
class B;
void FuncInB();
extern int variable_in_b;
ABSL_DECLARE_FLAG(flag_in_b);
```

**Pros:**

- 由於 #include 會強制編譯器開啟更多的檔案與處理更多的輸入，利用前置宣告減少 #include 可以減少編譯時間。
- 越多的 #include 代表程式碼更可能因為相依的標頭檔更動而被重新編譯，使用前置宣告可以節省不必要的重新編譯。

**Cons:**

- 前置宣告可能隱藏掉與標頭檔間的相依關係，導致當標頭檔改變時，相依的程式碼沒有被重新編譯。
- 使用前置宣告可能會使某些 IDE 自動工具難以找到其定義。
- 前置宣告可能在函式庫進行改動時發生編譯錯誤。例如函式庫開發者放寬了某個參數類型、替樣板增加預設參數或更改命名空間等等。
- 前置宣告來自 std:: 命名空間的 symbols 會導致未定義行為 (undefined behavior)。
- 難以抉擇是要使用前置宣告或是引入完整得標頭檔。在某些狀況下，使用前置宣告替換掉 #include 可能意外的修改了程式碼的意圖。如下，若 #include 被替換成 B 和 D 的前置宣告，test() 會呼叫到 f(void*)。
  
    ```cpp
    // b.h:
    struct B {};
    struct D : B {};

    // good_user.cc:
    #include "b.h"
    void f(B*);
    void f(void*);
    void test(D* x) { f(x); }  // Calls f(B*)
    ```

- 使用前置宣告多個 symbols 可能暴露了比直接引入標頭檔更多的訊息。
- 為了使用前置宣告而修改程式碼（例如：使用指標成員而不是物件成員) 可能會導致程式運作較為緩慢或是更加的複雜。

**Decision:**

在任何狀況下避免使用前置宣告。

### 5. Inline Functions

> **只有當函式非常的短，例如只有 10 行甚至更少的時候，才將其定義為內聯函式。**

**Definition:**

當函式被宣告為內聯函式之後，代表你允許編譯器將其展開在該函式被呼叫的位置，而不是原來的函式呼叫機制進行。

**Pros:**

當函式主體比較小的時候，內聯該函式可以產生更有效率目標程式碼 (object code)。對於存取函式 (accessors)、賦值函式 (mutators) 以及其它函式體比較短或性能關鍵的函式，可以依據需求將其轉為內聯函式。

**Cons:**

濫用內聯反而會導致程式變慢。內聯可能使目標程式碼變大或變小，這取決於內聯函式主體的大小。一個非常短小的存取函式被內聯通常會減少目標程式碼的大小， 但內聯一個相當大的函式將戲劇性的增加目標程式碼大小。現代的處理器 (CPU) 具備有指令緩存 (instruction cache)，執行小巧的程式碼往往執行更快。

**Decision:**

一個較為合理的經驗準則是，不要內聯超過 10 行的函式。謹慎對待解構子, 解構子往往比其表面看起來要更長, 因為有隱含的成員和父類別解構子被呼叫！

另一個實用的經驗準則: 內聯那些包含 loop 或 switch 語句的函式常常是得不償失的 (除非在大多數情況下, 這些 loop 或 switch 語句從不被執行)。

要注意的是，既使函式即使宣告為內聯，也不一定會被編譯器內聯。例如虛函式 (virtual) 和遞迴函式 (recursive) 就不會被正常內聯。通常, 遞迴函式不應該宣告成內聯函式。虛函式內聯的主要原因則是想把它的函式主體放在類別的定義內， 可能式為了方便，或是當作文件描述其行為。例如存取函式或賦值函式。

### 6. Names and Order of Includes

> **使用以下標準的標頭檔引入順序可增強可讀性，同時避免隱藏相依性：相關標頭檔 > C 函式庫 > C++ 函式庫 > 其他函式庫的 .h > 專案內的 .h。**

專案內的標頭檔應按照專案目錄樹結構排列，避免使用 UNIX 特殊的目錄捷徑 . (當前目錄) 或 .. (上層目錄)。例如：`htc-awesome-project/src/base/logging.h` 應該按如下方式引入:

```cpp
#include "base/logging.h"
```

另一個例子是，若 `dir/foo.cc` 或 `dir/foo_test.cc` 的主要作用是實作或測試 `dir2/foo2.h` 的功能，`foo.cc` 中引入標頭檔的次序應如下：

1. `"dir2/foo2.h"`
2. A blank line
3. C 系統文件，e.g., `<unistd.h>`, `<stdlib.h>`
4. A blank line
5. C++ 系統文件，e.g., `<algorithm>`, `<cstddef>`
6. A blank line
7. 其他函式庫的 `.h` 文件
8. A blank line
9. 此專案內 `.h` 文件

標頭檔的順序在依照類別分類後，同類別的引入順序則應該依照按字母順序排列。

使用這種排序方式，若 `dir2/foo2.h` 忽略了任何需要的標頭檔，在編譯 `dir/foo.cc` 或 `dir/foo_test.cc` 就會發生錯誤。因此這個規則可以確這些功能的保開發者可以在第一時間就發現錯誤。

舉例來說，`htc-awesome-project/src/foo/internal/fooserver.cc` 的引入次序如下：

```cpp
#include "foo/public/fooserver.h"

#include <sys/types.h>
#include <unistd.h>
#include <hash_map>
#include <vector>

#include "base/basictypes.h"
#include "base/commandlineflags.h"
#include "foo/public/bar.h"
```

例外：

有時，平台特定（system-specific）的程式碼需要依據條件被引入（conditional includes），這些程式碼可以放到其它的 includes 之後。當然，盡量讓你的平台特定程式碼小 (small) 且集中 (localized)，例如：

```cpp
#include "foo/public/fooserver.h"

#include "base/port.h"  // For LANG_CXX11.

#ifdef LANG_CXX11
#include <initializer_list>
#endif  // LANG_CXX11
```

---

## Scoping

### 7. Namespaces

> **除了少數的例外，都建議使用把程式碼放在命名空間內。一個具名的命名空間應該擁有唯一的名字，其名稱可基於專案名稱，甚至是相對路徑。而在 .cc 文件內，使用匿名的命名空間是推薦的，但禁止使用 using 指示（using-directives）和內聯命名空間（inline namespaces）。**

**Definition:**

命名空間將全域作用域細分為獨立的，具名的作用域可有效防止全域作用域的命名衝突。

**Pros:**

命名空間可以在大型專案內避免名稱衝突，同時又可以讓多數的程式碼有合理簡短的名稱。

舉例來說, 兩個不同專案的全域作用域都有一個類別 `Foo`，這樣在編譯或運行時期會造成衝突。如果每個專案將程式碼置於不同命名空間中，`project1::Foo` 和 `project2::Foo` 在專案中就可以被視為不同的 symbols 而不會發生衝突。兩個類別在各自的命名空間中，也可以繼續使用 `Foo` 而不需要前綴命名空間。

內聯命名空間會自動把內部的標識符放到外層作用域，比如：

```cpp
namespace outer {
inline namespace inner {
  void foo();
}  // namespace inner
}  // namespace outer
```

`outer::inner::foo()` 與 `outer::foo()` 彼此可以互換使用。內聯命名空間主要用來保持跨版本的 ABI 相容性。

**Cons:**

命名空間可能造成疑惑，因為它增加了識別一個名稱所代表的意涵的難度。例如： `Foo` 是命名空間或是一個類別。

內聯命名空間更是容易令人疑惑，因為它並不完全符合命名空間的定義；內聯命名空間只在大型版本控制裡會被使用到。

在標頭檔中使用匿名命名空間容易導致違背 C++ 的唯一定義原則 (One Definition Rule (ODR))。

在某些狀況中，經常會需要重複的使用完整 (fully-qualified) 的名稱來參考某些 symbols。對於多層巢狀的命名空間，這會增加許多混亂。

**Decision:**

使用命名空間如下：

- 遵循 [Namespace 命名規則](#50-namespace-names)
- f
- Namespace 應包含整個 source file，並且排在來自其他 namespaces 的 includes 和前置宣告之後
  
    ```cpp
    // In the .h file
    namespace mynamespace {

    // All declarations are within the namespace scope.
    // Notice the lack of indentation.
    class MyClass {
    public:
    ...
    void Foo();
    };

    }  // namespace mynamespace
    ```

    ```cpp
    // In the .cc file
    namespace mynamespace {

    // Definition of functions is within scope of the namespace.
    void MyClass::Foo() {
    ...
    }

    }  // namespace mynamespace
    ```

    更複雜的例子：

    ```cpp
    #include "a.h"

    ABSL_FLAG(bool, someflag, false, "a flag");

    namespace mynamespace {

    using ::foo::Bar;

    ...code for mynamespace...    // Code goes against the left margin.

    }  // namespace mynamespace
    ```

- 禁止在 `std` 命名空間中定義東西
- 禁止使用 using-directives，這會污染命名空間。

    ```cpp
    // Forbidden -- This pollutes the namespace.
    using namespace foo;
    ```

- 禁止使用內聯命名空間
- 禁止在 `.h` 中使用命名空間別名 (Namespace aliases)，但在 `.cc` 中允許。因為若在 `.h` 將會成為 API 的一部份洩露給所有人。
  
    ```cpp
    // Shorten access to some commonly used names in .cc files.
    namespace baz = ::foo::bar::baz;
    ```

### 8. Internal Linkage

> **當`.cc`檔案中的定義不需要在該檔案之外引用時，將它們放置在匿名命名空間來賦予它們內部連結 (Internal Linkage)。 禁止在`.h`檔案中使用。**

**Definition:**

所有宣告可以放置在匿名命名空間來賦予他內部連結。同樣的，我們也可透過為 function 與 variables 加上 `static` 來賦予其性質。一旦賦予內部連結性質，所有該檔案以外的地方均無法參考。

**Decision:**

建議將任何不需給外部參考的東西放置在匿名空間中，這裡不推薦使用 `static` 的原因在於 `static` 在不同地方往往含意不同，這種寫法易造成混淆。禁止在`.h`檔案中使用內部連結。

```cpp
namespace {
int i = 20;
}

int main(int, char**) {
  std::cout << "i: " << ::i << std::endl;
}
```

### 9. Nonmember, Static Member, and Global Functions

> **建議將非成員函式放置在命名空間中，盡量不要使用完全的全域函式。建議利用命名空間來放置相關的多個函式，而不是全部放置在類別中並宣告成 `static`。類別的靜態方法一般來說要和類別的實例或類別的靜態資料有緊密的關連。**

**Pros:**

某些情況下，非成員函式和靜態成員函式是非常有用的。將非成員函式放在命名空間內可避免對於全域作用域污染。

**Cons:**

為非成員函式和靜態成員函式準備一個新的類別可能更有意義，特別是它們需要存取外部資源或式有大量的相依性關係時。

**Decision:**

有時候定義一個不綁定特定類別實例的函式是有用的，甚至是必要的。這樣的函式可以被定義成靜態成員或是非成員函式。非成員函式不應該依賴於外部變數，且應該總是放置於某個命名空間內。相比單純為了封裝不共享任何靜態數據的靜態成員函式而創建一個類別，不如之直接使用 [Namespaces](#7-namespaces)。例如對於 myproject/foo_bar.h 標頭擋來說，可以這樣寫。

```cpp
namespace myproject {
namespace foo_bar {
void Function1();
void Function2();
}
}
```

而不是

```cpp
// Forbidden
namespace myproject {
class FooBar {
 public:
  static void Function1();
  static void Function2();
};
}
```

如果你必須定義非成員函式，又只是在 `.cc` 文件中使用它，則可使用 [Internal Linkage](#8-internal-linkage) 限定其作用域。

### 10. Local Variables

> **盡可能將函式內的變數的作用域最小化，並在變量宣告時進行初始化。**

C++ 允許在函式內的任何位置宣告變數。我們鼓勵在盡可能小的作用域中宣告變量，並且離第一次使用的地方越近越好。這會讓閱讀者更容易找到變數宣告的位置、宣告的類型和初始值。要注意，應該該宣告時直接初始化變數，而不要先代宣告再後賦值, 例如：

```cpp
int i;
i = f();      // Bad -- initialization separate from declaration.
```

```cpp
int i = f();  // Good -- declaration has initialization.
```

```cpp
int jobs = NumJobs();
// More code...
f(jobs);      // Bad -- declaration separate from use.
```

```cpp
int jobs = NumJobs();
f(jobs);      // Good -- declaration immediately (or closely) followed by use.
```

```cpp
std::vector<int> v = {1, 2};  // Good -- v starts initialized.
```

在 `if`、`while`和`for`陳述句需要的變數一般都會宣告在這些陳述句中，也就是這些變數會存活於這些作用域內。例如：

```cpp
while (const char* p = strchr(str, '/')) str = p + 1;
```

如果變數是一個物件，每次進入作用域時其建構子都會被呼叫，每次離開作用域時其解構子都會被呼叫。

```cpp
// Inefficient implementation:
for (int i = 0; i < 1000000; ++i) {
  Foo f;  // My ctor and dtor get called 1000000 times each.
  f.DoSomething(i);
}
```

在循環作用域外面宣告這類型的變數可能更加的有效率。

```cpp
Foo f;  // My ctor and dtor get called once each.
for (int i = 0; i < 1000000; ++i) {
  f.DoSomething(i);
}
```

### 11. Static and Global Variables

> **除非物件是 trivially destructible，不然禁止使用具有 static storage duration 的物件。Static function-local variables 可以使用動態初始化 (dynamic initialization)。 不鼓勵對 namespace 內的變數或者 static class member variables 或進行動態初始化。**
> **作為經驗法則：一個宣告為 `constexpr` 的變數、[POD (Plain Old Data)](https://zh.wikipedia.org/zh-tw/POD_(程序设计)) 如 `int` `char` `float` `raw pointer`、POD array/struct/class ，可以滿足上述要求。**

- [trivially destructible](https://cplusplus.com/reference/type_traits/is_trivially_destructible/): 基本上指 POD。
- [static storage duration](https://en.cppreference.com/w/c/language/static_storage_duration): 物件的生命週期(lifetime)是從程式開始執行的時候開始，程式結束之後才會被釋放。
- [dynamic initialization](https://stackoverflow.com/questions/5945897/what-is-dynamic-initialization-of-object-in-c): 初始值無法在 compile-time 得知，會在 runtime 時計算。

**Definition:**

每個物體都有一個 storage duration，這與其生命週期相關。 具有 static storage duration 的物件從初始化開始一直持續到程式結束。 此類物件在命名空間範圍內（“全域性變數”）作為變數出現，或作為 static data members of classes，或作為 static function-local variables 出現。Static function-local variables 會在首次呼叫該函式時初始化；而其他類型皆在程式啟動時初始化。 所有具有static storage duration 的物件都會在程式退出時被銷毀（注意：發生在 unjoined threads 被終止之前）。

Dynamic initialization，代表在初始化期間會執行 non-trivial 運算（例如，allocates memory, 變數由 current PID 初始化），也因此無法在 complie-time 得知，反之則是 Static initialization。 Static initialization 總是發生在具有 static storage duration 的物件身上，動態初始化會發生在 runtime。

**Pros:**

Global and static variables 對多數應用程式非常有用：named constants, auxiliary data structures internal to some translation unit, command-line flags, logging, registration mechanisms, background infrastructure, etc.。

**Cons:**

使用 dynamic initialization 或具有 non-trivial destructors 的全域性和靜態變數很容易導致難以找到的錯誤。 原因在於，大型專案很難控制個單元的連結順序，建構子、解構子和初始化的順序在 C++ 中規範並不完整，導致每次編譯會產生不同的結果。 當一個靜態變數初始化時使用另一個變數，這可能會導致物件在其生命週期開始前（或其生命週期結束後）被訪問。 此外，當程式啟動 unjoined threads 並且未在結束前 join，這些執行緒可能會在其生命週期結束後嘗試訪問物件。

**Decision:**

- **Decision on destruction**
  
  當 destructors 為 trivial，它們的執行完全不受順序約束（它們實際上不會“執行”）；否則，我們將面臨在物件生命週期結束後訪問物件的風險。 因此，我們只允許具有 static storage duration 的物件，前提是它們是 trivially destructible。 如 POD， 標有 `constexpr` 的變數。

  ```cpp
  const int kNum = 10;  // Allowed

  struct X { int n; };
  const X kX[] = {{1}, {2}, {3}};  // Allowed

  void foo() {
    static const char* const kMessages[] = {"hello", "world"};  // Allowed
  }

  // Allowed: constexpr guarantees trivial destructor.
  constexpr std::array<int, 3> kArray = {1, 2, 3};
  ```

  ```cpp
  // bad: non-trivial destructor
  const std::string kFoo = "foo";

  // Bad for the same reason, even though kBar is a reference (the
  // rule also applies to lifetime-extended temporary objects).
  const std::string& kBar = StrCat("a", "b", "c");

  void bar() {
    // Bad: non-trivial destructor.
    static std::map<int, int> kData = {{1, 0}, {2, 0}, {3, 0}};
  }
  ```

  請注意，references 不是物件，雖然因此不受 destructibility 的限制，但還是受 dynamic initialization 影響。唯一的例外為：`static T& t = *new T`，這種寫法可被允許。

- **Decision on initialization**
  
  初始化更為複雜，端看 constructors 的設計，考慮以下全域變數的寫法：

  ```cpp
  int n = 5;    // Fine
  int m = f();  // ? (Depends on f)
  Foo x;        // ? (Depends on Foo::Foo)
  Bar y = g();  // ? (Depends on g and on Bar::Bar)
  ```

  若使用 constant initialization，則可被允許：

  ```cpp
  struct Foo { constexpr Foo(int) {} };

  int n = 5;  // Fine, 5 is a constant expression.
  Foo x(2);   // Fine, 2 is a constant expression and the chosen constructor is constexpr.
  Foo a[] = { Foo(1), Foo(2), Foo(3) };  // Fine
  ```

  任何沒有如此標記的 non-local static storage duration variable 都應推定為具有動態初始化，並非常仔細地審查。
  相反的，以下不被允許：

  ```cpp
  // Some declarations used below.
  time_t time(time_t*);      // Not constexpr!
  int f();                   // Not constexpr!
  struct Bar { Bar() {} };

  // Problematic initializations.
  time_t m = time(nullptr);  // Initializing expression not a constant expression.
  Foo y(f());                // Ditto
  Bar b;                     // Chosen constructor Bar::Bar() not constexpr.
  ```

- **Common patterns**
  以下將討論各種需要設為全域或靜態變數的情境：
  - **Global `string`**: 若要使用，考慮宣告成 `constexpr` 的 `string_view` （string_view 具有 constexpr constructor 和 trivial destructor），`char[]`，`char*`。
  - **Maps, sets, and other dynamic containers**：任何 dynamic contianers 都不具有 trivial destructor，因此不被允許使用。請考慮用 array of array 或者 array of pair 來取代 （當然，array 裡面也要是 POD）。宣告時可考慮將值排序，以便進行搜尋時使用 binary search 增加效率。
  - **Smart pointers**：smart pointers 不具有 trivial destructor 因此不被允許。若要使用 pointer，直接宣告一個 `raw pointer` 即可。
  - **Static variables of custom types**：若要使用，則需定義對應的 `constexpr` constructor，並且需具有 trivial destructor。
  - 若上述方式都無法滿足需求，還有一種特殊方式可以動態宣告，考慮使用 function-local static pointer / reference:
  
    ```cpp
    T& GetT() {
      static const auto& impl = *new T(args...);
      retrun impl;
    }
    ```

### 12. `thread_local` Variables

> **所有未在 function 內宣告的 `thread_local` 變數，必須宣告為 `constexpr` 。任何 thread-local data 請偏好使用 `thread_local`。**

**Definition:**

變數可以宣告為 `thread_local`:

```cpp
thread_local Foo foo = ...;
```

這種變數實際上是物件的集合，因此當不同的執行緒訪問它時，它們實際上是在訪問不同的物件。`thread_local` 可看作為單一 thread 範疇內的  [static storage duration variables](#11-static-and-global-variables)，例如：他可以宣告在 namespace，可以在 function 內，或者為 static class members，但不可作為一般的 class members。

`thread_local` 變數初始化與靜態變數非常相似，只是它們必須為每個執行緒單獨初始化，而不是在程式啟動時初始化一次。 這意味著函式中宣告的 `thread_local` 變數是安全的，但在其他地方宣告的 `thread_local` 變數會受到與靜態變數相同的初始化順序問題（以及更多）。

`thread_local` 變數有一個微妙的銷毀順序問題：在執行緒關閉期間， `thread_local` 變數將按照其初始化的相反順序被銷毀。 如果由任何 `thread_local` 變數的 destructor 中訪問任何已被破壞的 `thread_local`，將產生難以發現的 use-after-free bug。

**Pros:**

- Thread-local data 能有效避免 data racing 問題，適合用於 concurrent programming.
- `thread_local` 是唯一一個 c++ 標準支持創建 thread-local data 的方式。

**Cons:**

- 每當新的 thread 開始，`thread_local`變數的 constructor 也無可避免的需要被重新運算。
- `thread_local` 變數是一種全域變數，因此也同樣有全域變數的所有缺點（但他會是 thread-safety）。
- 隨著 thread 數量上升，`thread_local` 變數所佔用的記憶體也隨之上升。
- 一般的 data member 不能為 `thread_local` 除非他是 `static`。
- 有可能發生 use-after-free bug，如上所述。

**Decision:**

當想要在 namespace 或者 class scope 中宣告 `thread_local` 變數，請使其為一個 `constexpr`，或者使用 function-local 技巧：

```cpp
constexpr thread_local Foo foo = ...;

// or
Foo& MyThreadLocalFoo() {
  thread_local Foo result = ComplicatedInitialization();
  return result;
}
```

在函式內宣告的 `thread_local` 變數則沒有初始化問題，但也會有潛在的 use-after-free bug 風險。實務上，請考慮使用 trival data types，或者在 destructor 中避免使用自定義的 class，以免訪問到其他的 `thread_local` 變數。

當有使用 thread-local data 的需求，請使用 `thread_local` 取代自定義的機制。

---

## Classes

類別是 C++ 中程式碼的基本單元。想當然爾, 在程式中類別將被廣泛使用。本節列舉了在撰寫一個類別時該做的和不該做的事項.

### 13. Doing Work in Constructors

> **不要在建構子中呼叫虛函式 (virtual function)，也不要做任何有失敗可能的運算.**

**Definition:**

在建構子體中進行初始化操作.

**Pros:**

- 無須擔心此類別是否已經初始化.
- 物件由建構子初始化可賦予 `const`，也可以方便使用於 standard containers 或者演算法中.

**Cons:**

- 如果在建構子內呼叫了自身的虛函式, 這類呼叫是不會重定向 (dispatched) 到子類的虛函式實作. 即使當前沒有子類化實作, 將來仍是隱患.
- 建構子中難以報錯, 或使用例外.
- 建構失敗會造成對象進入不確定狀態．或許可使用類似 `IsValid()` 的機制去做狀態檢查，但這不具強制性也很容忘記使用．
- 建構子無法獲得地址，意味著無法移交至 thread 中運算。

**Decision:**

建構子不得呼叫虛函式。 若物件初始化過於複雜，有潛在失敗的風險, 考慮使用明確的`Init()`方法以便進行錯誤處理，或更進一步使用工廠模式包裝，防止物件未正確初始化：

```cpp
// foo.h
class Foo {
 public:
  // Factory method: creates and returns a Foo.
  // May return null on failure.
  static std::unique_ptr<Foo> Create();

  // Foo is not copyable.
  Foo(const Foo&) = delete;
  Foo& operator=(const Foo&) = delete;

 private:
  // Clients can't invoke the constructor directly.
  Foo();
  void Init();
};

// foo.c
std::unique_ptr<Foo> Foo::Create() {
  // Note that since Foo's constructor is private, we have to use new.
  auto inst = std::make_unique<Foo>();
  inst.Init(); // Ensure proper initialization
  return std::move(inst);
}
```

### 14. Implicit Conversions

> **不要定義隱式轉換（Implicit conversion）。對單個參數的建構子與 conversion operator (e.g., `operator bool()`) 使用 C++ 關鍵字 `explicit`。**

**Definition:**

隱式轉換允許將 source type 轉換為 destination type，如將 `int` 傳入接受 `double` 參數的函式。

除此之外，透過 conversion operator，C++ 允許自定義的隱式轉換。而當自定義的 constructor 為單參數的建構子時（或者只有一個參數沒有預設值，e.g., `Foo::Foo(string name, int id = 42)`），隱式轉換也會發生。

`explicit` 可使用在建構子與 conversion operator ，確保了 destination type 必須與 source type 一致。此效用不但防止隱式轉換，同時也作用於 list initialization。如下例：

```cpp
class Foo {
  explicit Foo(int x, double y);
  ...
};

void Func(Foo f);
```

```cpp
Func({42, 3.14});  // Error
```

**Pros:**

- 隱式轉換可以增加便利性，不必做明確的轉型（type casting）。
- 隱式轉換可以成為 overloading 的簡單替代方案，例如當帶有 `string_view` 引數的函式也可接收 `std::string` 和 `const char*` ，不必再定義重載。
- 列表初始化（list initialization）語法是初始化物件的一種簡潔方式。

**Cons:**

- 隱式轉換會造成隱藏型別不匹配錯誤，或者使用者不知道將發生何種轉換。
- 隱式轉換可以使程式更難閱讀，特別是在 overloading 的情況下，因為無法判定何種函式被呼叫。
- 單參數的建構函式可能會意外地用作隱式型別轉換，即使不打算這樣做。
- 當單參數構造函式沒有標記為 `explicit` 時，沒有可靠的方法來判斷它是否旨在定義隱式轉換，還是作者只是忘記標記它。
- 隱式轉換可能會導致呼叫模糊性，特別是當有雙向隱式轉換時。 這發生於由兩種提供隱式轉換的型別引起的，也可以是由同時具有隱式建構子和 conversion operator 的型別造成的。
- 如果目標型別是隱式的，特別是如果列表只有一個元素，列表初始化可能會遇到同樣的問題。

**Decision:**

所有單參數建構子與 conversion operator 都必須是顯式的。但 copy ＆ move constructors 不能為顯式, 因為他們不涉及型別轉換.

例外: 當物件可用不同型別表示，其背後的值相同時，可與你的 leader 討論是否可忽略此規則。

最後, 只有 `std::initializer_list` 的建構子可以是非 `explicit`, 以允許你的類型結構可以使用列表初始化的方式進行賦值. 例如: `MyType m = {1, 2};`。

### 15. Copyable and Movable Types

> **Class 的 public API 必須明確宣告該類是 copyable, move-only, 或者都不是。 如果這些操作對您的型別來說清楚且有意義，請支援 copy 和/或 move。**

**Definition:**



**Pros:**

**Cons:**

**Decision:**

### 16. Structs vs. Classes

> 僅當只有數據時使用 `struct`，其它一概使用 `class`。

**Decision:**

在 C++ 中 `struct` 和 `class` 關鍵字幾乎含義一樣. 我們為這兩個關鍵字添加我們自己的語義理解, 以便在定義數據類型時選擇合適的關鍵字.

`struct` 用來定義包含僅包含數據的對象, 也可以包含相關的常數, 但除了 getter/setter 之外, 沒有別的函式功能。 所有數據皆為 `public`，並且僅允許建構子, 解構子, Operator, 與相關的 helper function。 所有變數與函式應避免引入不變性 (invariants)。

如果需要更多的函式功能，`class` 更適合。如果拿不準，就用 `class`。

為了和 STL 保持一致, 對於 stateless types 的特性可以不用 `class` 而是使用 `struct`，像是 [traits](https://stephlin.github.io/post/cpp/cpp-traits/), [template metafunctions]((https://google.github.io/styleguide/cppguide.html#Template_metaprogramming)), etc.

注意: `class` 和 `struct` 的成員變數使用不同的命名規則.

### 17. Structs vs. Pairs and Tuples

> 使用 `struct`，而非 `pair` 或者 `tuple`.

**Decision:**

當使用 `pair` 或者 `tuple` 時對於程式撰寫者有很大的方便性，然而對於閱讀者而言卻造成不便．光看 `.first`, `.second`, 或 `std::get<X>` 無法清楚的知道順序對應的含義，閱讀者必須移至宣告處才能明白．相反的，使用 `struct` 我們可以直接透過命名理解，更加增進閱讀效率．

### 18. Inheritance

> **多用組合，少用繼承。 使用繼承時，定義為 `public`。**

**Definition:**

**Pros:**

**Cons:**

**Decision:**

### 19. Operator Overloading

> dj/

**Definition:**

**Pros:**

**Cons:**

**Decision:**

### 20. Access Control

> **將所有數據成員宣告為 `private`，除非他是 `const`。 並根據需求提供相應的存取函式。**

將變數設為 private 可以有效地防止非預期的修改。命名規則為，某個名為 `foo_` 的變數，其取值函式是 `foo()`。賦值函式是 `set_foo()`。一般在標頭檔中把存取函式定義成 inline function。

### 21. Declaration Order

> **類的訪問控制區段的宣告順序依次為: `public:`，`protected:`，`private:`。如果某區段沒內容，不宣告。**

每個區段內的宣告按以下順序:

- Types and type aliases (typedef, using, enum, nested structs and classes, and friend types)
- (Optionally, for structs only) non-static data members
- Static constants
- Factory functions
- Constructors and assignment operators
- Destructor
- All other functions (static and non-static member functions, and friend functions)
- All other data members (static and non-static)

不要在類中定義大型 inline function。通常，只有那些沒有特別意義或性能要求高, 並且是比較短小的函式才能被定義為 [inline function](#5-inline-functions)。

---

## Functions

### 22. Inputs and Outputs

C++函式的輸出是透過 `return` 提供的，有ㄅ時也可透過 output parameters（or in/out parameters）。

盡可能的使用 `return` 而非 output parameters：它們提高了可讀性，並且通常提供相同或更好的效能 (現在編譯器都有 RVO 優化，更多請參考 [copy elision](https://en.cppreference.com/w/cpp/language/copy_elision) 的解釋，或者此篇文章：[Revisiting output parameters usefulness (in C++)](https://bulldogjob.pl/readme/revisiting-output-parameters-usefulness-in-c))。

請使用 return by value 或者 return by reference。 禁止使用 return by pointer（smart pointer 可被允許，並且可為 `nullptr`）。

Parameters 要麼是 input parameters，要麼是 output parameters，要麼兩者兼具：

- **Input paramters**: 請 pass by const reference/value，禁止使用 optional input，除非他用於 debug（請參考：[Avoid Passing Booleans to Functions](https://alexkondov.com/should-you-pass-boolean-to-functions/)）。若使用 optional input，請盡可能的使用 `std::optional` 以增加可讀性。
- **Output parameters & Input/Output parameters**: 若有需要使用此，請 pass by reference。禁止使用 optional output，除非他用於 debug。若使用 optional output，請盡可能的使用 `std::optional` 以增加可讀性。

在使用 pass by const reference 時，避免定義 const reference parameter 比 function call 生命週期長的函式，因為 const reference parameter 可以連結到 `rvalue`，看下面例子:

```cpp
class StringHolder {
public:
  // The input `val` must live as long as this object,
  // not just the call to this constructor
  StringHolder(const string& val) : val_(val) {}

  const string& get() { return val_; }
private:
  const string& val_;
}

//----

StringHolder holder("abc"s); // temporaries bind to const-ref 
std::cout << holder.get();  // boom, UB. 
// The string temporary has already been destroyed, the reference is dangling.

```

相反，找到一種方法來消除生命週期需求（例如，透過 pass by value）。

排序函式 parameters 時，將所有 input parameters 排在 output parameters 之前。 

### 23. Write Short Functions

> **盡可能寫小而集中的功能，並且遵循：一個函式只做一件事。**

如果一個函式超過大約40行，請考慮是否可以在不損害程式結構的情況下將其分解。

即使您的大函式現在工作正常，在幾個月內修改它的人可能會新增新行為。 這可能會導致難以找到的錯誤。 保持函式的簡短和只做一件事的原則，使其他人更容易閱讀和修改您的程式。**小函式也更容易測試**。

### 24. Functions Overloading

> **當變體之間沒有語義差異時，您可以重載函式。避免讓使用者呼叫時難以猜測實際上被呼叫的函式。**

**Definition:**

您可以編寫一個接受 `const std::string&` 的函式，並用另一個接受 `const char*` 的函式重載它。 (然而，在這種情況下，請考慮 `std::string_view`。)

```cpp
class MyClass {
 public:
  void Analyze(const std::string &text);
  void Analyze(const char *text, size_t textlen);
};
```

**Pros:**

透過允許同名函式接受不同的引數，overloading 可以使程式更直觀。 它可能需要模板化程式，並且對使用者來說很方便。

基於const或ref資格的超載可能會使實用程式更可用、更高效，或兩者兼而有之。 （有關更多資訊，請參閱TotW 148。）

**Cons:**

如果函式 overloading 僅針對 parameter type 變化，讀者可能必須瞭解C++的複雜匹配規則才能知道發生了什麼。 如果 derived class 只覆蓋函式的一些 variants，許多人也會對繼承的語義感到困惑。

**Decision:**

當變體之間沒有語義差異時，您可以重載函式，否則考慮定義新的函式。 這些重載可能因型別、qualifiers 或 parameters 數量而異。如果您可以在 header 中用簡單的 comment 記錄 overload set 中的所有變體，這是一個好跡象，表明它是一個設計良好的 overload set。


### 25. Default Arguments

> **預設值只被允許用於 non-virtual function，因為這保證始終具有相同的值。 遵循與 Function overloading 相同的限制，如果預設引數獲得的可讀性沒有比以下缺點更有價值，則考慮重載函式。**

**Pros:**

通常，您有一個使用預設值的函式，但偶爾您想覆蓋預設值。 預設引數允許一種簡單的方法來做到這一點，而無需為罕見的異常定義許多函式。 與重載函式相比，預設引數具有更清晰的語法，樣板更少，並且“必需”和“可選”引數之間的區別更清晰。

**Cons:**

預設引數是實現 function overloading 的另一種方式，因此所有 [function overloading 的缺點](#24-functions-overloading) 都適用。

Virtual function 呼叫中 parameter 的預設值由目標物件的靜態型別決定，不能保證給定函式的所有 override 都宣告相同的預設值，例如：

```cpp
struct A {
    virtual void display(int i = 5) { std::cout << "Base::" << i << "\n"; }
};
struct B : public A {
    virtual void display(int i = 9) override { std::cout << "Derived::" << i << "\n"; }
};

int main()
{
    A * a = new B();
    a->display(); // Derived::5

    A* aa = new A();
    aa->display(); // Base::5

    B* bb = new B();
    bb->display(); // Derived::9
}
```

預設引數在每個呼叫點重新計算，這可能會使生成的程式膨脹。 使用者可能還期望預設值在宣告時固定，而不是在每次呼叫時變化。

存在預設引數時，函式指標會令人困惑，因為函式 signature 通常與呼叫 signature 不匹配。 新增 function overloading 可以避免這些問題。

**Decision:**

預設引數在 virtual function 上被禁止，因為在指定的預設值可能不計算為相同值。

在其他一些情況下，預設引數可以提高其函式宣告的可讀性，足以克服上述缺點，因此允許它們。 當有疑慮時，請使用 overloading。

### 26. Trailing Return Type Syntax

> **僅在典型的語法難以閱讀時才使用 trailing return type syntax。**

**Definition:**

C++允許兩種不同形式的函式宣告：

```cpp
int foo(int x);
auto foo(int 2) -> int; // trailing return type syntax
```

**Pros:**

Trailing return type 是顯式指定 [lambda表示式](#36-lambda-expressions) 的返回型別的唯一方法。 在某些情況下，編譯器能夠推斷lambda 的返回型別，但並非在所有情況下。 即使編譯器可以自動推斷它，有時明確指定它對讀者來說會更清晰。

有時，在函式的 parameter list 已經宣告候，指定返回型別更容易，也更容易閱讀。 當返回型別取決於模板引數時，情況尤其如此。 例如：

```cpp
template <typename T, typename U>
auto add(T t, U u) -> decltype(t + u);
```

對比

```cpp
template <typename T, typename U>
decltype(declval<T&>() + declval<U&>()) add(T t, U u);
```

**Cons:**

Trailing return type syntax 相對較新，因此一些讀者可能會不熟悉。

現有的程式庫有大量的函式宣告，這些宣告不會被更改為使用新語法，因此現實的選擇是只使用舊語法或使用兩者的混合。 使用單一版本更適合風格的一致性。

**Decision:**

在大多數情況下，繼續使用舊的函式宣告樣式，返回型別在函式名稱之前。 僅在需要時（如lambdas）或透過將型別放在函式引數列表後，允許您以更可讀的方式編寫型別時使用 trailing return type。 後一種情況應該很罕見；這在很大程度上是一個相當複雜的模板程式中的問題，在大多數情況下不鼓勵這樣做。

---

## C++ Features

### 27. Ownership and Smart Pointers

### 28. Rvalue References

### 29. Friends

### 30. Exceptions

### 31. noexcept

### 32. Run-Time Type Information (RTTI)

### 33. Casting

### 34. Streams

### 35. Preincrement and Predecrement

### 36. Use of const

### 37. Use of constexpr, constinit, and consteval

### 38. Integer Types

### 39. 64-bit Portability

### 30. Preprocessor Macros

### 31. 0 and nullptr/NULL

### 32. sizeof

### 33. Type Deduction (including auto)

### 34. Class Template Argument Deduction

### 35. Designated Initializers

### 36. Lambda Expressions

### 37. Template Metaprogramming

### 38. Concepts and Constraints

### 39. Boost

### 40. Other C++ Features

### 41. Nonstandard Extensions

### 42. Aliases

### 43. Switch Statements

---

## Naming

### 44. General Naming Rules

### 45. File Names

### 46. Type Names

### 47. Variable Names

### 48. Constant Names

### 49. Function Names

### 50. Namespace Names

### 51. Enumerator Names

### 52. Macro Names

### 53. Exceptions to Naming Rules

---

## Comments

### 54. Comment Style

### 55. File Comments

### 56. Struct and Class Comments

### 57. Function Comments

### 58. Variable Comments

### 59. Implementation Comments

### 60. Punctuation, Spelling, and Grammar

### 61. TODO Comments

