# C++ Style Guide for SLAM

## Header Files

### 1. Self-contained Headers

### 2. The #define Guard

### 3. Include What You Use

### 4. Forward Declarations

### 5. Inline Functions

### 6. Names and Order of Includes

## Scoping

### 7. Namespaces

### 8. Internal Linkage

### 9. Nonmember, Static Member, and Global Functions

### 10. Local Variables

### 11. Static and Global Variables

### 12. Thread local Variables

## Classes

類別是 C++ 中程式碼的基本單元。想當然爾, 在程式中類別將被廣泛使用。本節列舉了在撰寫一個類別時該做的和不該做的事項.

### 13. Doing Work in Constructors

> 不要在建構子中呼叫虛函式 (virtual function)，也不要做任何有失敗可能的運算.

**Definition:**

在建構子體中進行初始化操作.

**Pros:**

- 無須擔心此類別是否已經初始化.
- 物件由建構子初始化可賦予 `const`，也可以方便使用於 standard containers 或者演算法中.

**Cons:**

- 如果在建構子內呼叫了自身的虛函式, 這類呼叫是不會重定向 (dispatched) 到子類的虛函式實作. 即使當前沒有子類化實作, 將來仍是隱患.
- 建構子中難以報錯, 或使用例外.
- 建構失敗會造成對象進入不確定狀態．或許可使用類似 `IsValid()` 的機制去做狀態檢查，但這不具強制性也很容忘記使用．
- 如果有人創建該類型的全域變數 (雖然違背了上節提到的規則), 建構子將先 ``main()`` 一步被呼叫, 有可能破壞建構函式中暗含的假設條件. 例如, `gflags <http://code.google.com/p/google-gflags/>`_ 尚未初始化.

**Decision:**

建構子不得呼叫虛函式, 或嘗試報告一個非致命錯誤. 如果對象需要進行有意義的 (non-trivial) 初始化, 考慮使用明確的`Init()`方法或使用工廠模式.

### 14. Implicit Conversions

> 對單個參數的建構子使用 C++ 關鍵字 ``explicit``.

**Definition:**

通常, 如果建構子只有一個參數, 可看成是一種隱式轉換. 打個比方, 如果你定義了 ``Foo::Foo(string name)``, 接著把一個字符串傳給一個以 ``Foo`` 對象為參數的函式, 建構函式 ``Foo::Foo(string name)`` 將被呼叫, 並將該字符串轉換為一個 ``Foo`` 的臨時對像傳給呼叫函式. 看上去很方便, 但如果你並不希望如此通過轉換生成一個新對象的話, 麻煩也隨之而來. 為避免構造函式被呼叫造成隱式轉換, 可以將其宣告為 ``explicit``.

除單參數建構子外, 這一規則也適用於除第一個參數以外的其他參數都具有默認參數的建構函式, 例如 Foo::Foo(string name, int id = 42).

**Pros:**

無

**Cons:**

無

**Decision:**

所有單參數建構子都必須是顯式的. 在類定義中, 將關鍵字 ``explicit`` 加到單參數建構函式前: ``explicit Foo(string name);``

例外: 在極少數情況下, 拷貝建構子可以不宣告成 ``explicit``. 作為其它類的透明包裝器的類也是特例之一. 類似的例外情況應在註解中明確說明.

最後, 只有 std::initializer_list 的建構子可以是非 explicit, 以允許你的類型結構可以使用列表初始化的方式進行賦值. 例如:

```cpp
MyType m = {1, 2};
MyType MakeMyType() { return {1, 2}; }
TakeMyType({1, 2}); 
```

### 15. Copyable and Movable Types

> dj/

**Definition:**

**Pros:**

**Cons:**

**Decision:**

### 16. Structs vs. Classes

> 僅當只有數據時使用 `struct`, 其它一概使用 `class`.

**Decision:**

在 C++ 中 `struct` 和 `class` 關鍵字幾乎含義一樣. 我們為這兩個關鍵字添加我們自己的語義理解, 以便在定義數據類型時選擇合適的關鍵字.

`struct` 用來定義包含僅包含數據的對象, 也可以包含相關的常數, 但除了存取數據成員之外, 沒有別的函式功能. 所有數據皆為 `public`，並且僅允許建構子, 解構子, Operator, 與相關的 helper function. 所有變數與函式應避免引入不變性 (invariants).

如果需要更多的函式功能, `class` 更適合. 如果拿不準, 就用 `class`.

為了和 STL 保持一致, 對於 stateless types 的特性可以不用 `class` 而是使用 `struct`，像是 [traits](https://stephlin.github.io/post/cpp/cpp-traits/), [template metafunctions]((https://google.github.io/styleguide/cppguide.html#Template_metaprogramming)), etc.

注意: 類和結構體的成員變數使用不同的命名規則.

### 17. Structs vs. Pairs and Tuples

> 使用 `struct`，而非 `pair` 或者 `tuple`.

**Decision:**

當使用 `pair` 或者 `tuple` 時對於程式撰寫者有很大的方便性，然而對於閱讀者而言卻造成不便．光看 `.first`, `.second`, 或 `std::get<X>` 無法清楚的知道順序對應的含義，閱讀者必須移至宣告處才能明白．相反的，使用 `struct` 我們可以直接透過命名理解，更加增進閱讀效率．

### 18. Inheritance

> dj/

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

> 將所有數據成員宣告為 `private`, 除非他是 `constant`. 並根據需要提供相應的存取函式. 命名規則為, 某個名為 `foo_` 的變數, 其取值函式是 `foo()`. 賦值函式是 `set_foo()`. 一般在標頭檔中把存取函式定義成 inline function.

### 21. Declaration Order

類的訪問控制區段的宣告順序依次為: `public:`, `protected:`, `private:`. 如果某區段沒內容, 不宣告．

每個區段內的宣告按以下順序:

- Types and type aliases (typedef, using, enum, nested structs and classes, and friend types)
- (Optionally, for structs only) non-static data members
- Static constants
- Factory functions
- Constructors and assignment operators
- Destructor
- All other functions (static and non-static member functions, and friend functions)
- All other data members (static and non-static)

不要在類中定義大型 inline function. 通常, 只有那些沒有特別意義或性能要求高, 並且是比較短小的函式才能被定義為 inline function. 更多細節參考 內聯函式.

## Functions

### 22. Inputs and Outputs

### 23. Write Short Functions

### 24. Functions Overloading

### 25. Default Arguments

### 26. Trailing Return Type Syntax

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

## Comments

### 54. Comment Style

### 55. File Comments

### 56. Struct and Class Comments

### 57. Function Comments

### 58. Variable Comments

### 59. Implementation Comments

### 60. Punctuation, Spelling, and Grammar

### 61. TODO Comments
