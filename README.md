# SmartPtrs 

Небольшой проект с реализациями основных умных указателей и вспомогательных примитивов управления временем жизни объектов:

- `UniquePtr` - эксклюзивное владение (аналог `std::unique_ptr`), включая специализацию для массивов.
- `SharedPtr` / `WeakPtr` - совместное владение с контрольным блоком (аналог `std::shared_ptr` / `std::weak_ptr`).
- `EnableSharedFromThis` - получение `SharedPtr`/`WeakPtr` на `this` (аналог `std::enable_shared_from_this`).
- `IntrusivePtr` - intrusive-счётчик ссылок (аналог `intrusive_ptr` из библиотеки Boost).

## Требования

- C++17 (используются `std::is_empty_v`, `std::is_final_v`, perfect forwarding).

## Структура

```

unique/
compressed_pair.h       # CompressedPair + EBO
unique.h                # UniquePtr + DefaultDeleter

shared_and_weak/
sw_fwd.h                # BadWeakPtr, объявления, ControlBlock + реализации
shared.h                # SharedPtr, EnableSharedFromThis, MakeShared
weak.h                  # WeakPtr

intrusive/
intrusive.h             # RefCounted/SimpleRefCounted, IntrusivePtr, MakeIntrusive

````

## UniquePtr

* владение одиночным объектом `UniquePtr<T>`
* владение массивом `UniquePtr<T[]>`
* кастомный deleter
* оптимизация размера через `CompressedPair` (EBO для пустых deleter)


## SharedPtr / WeakPtr


* контрольный блок с двумя счётчиками: `shared_count` и `weak_count`
* `MakeShared<T>(args...)` - single-allocation для объекта + control block
* aliasing-конструктор `SharedPtr(other, ptr)`
* `WeakPtr -> SharedPtr`:

  * `WeakPtr::Lock()` возвращает пустой `SharedPtr`, если объект уничтожен
  * `SharedPtr(const WeakPtr<T>&)` бросает `BadWeakPtr`, если weak истёк


### EnableSharedFromThis

Если тип `T` наследуется от `EnableSharedFromThis<T>`, то при создании `SharedPtr<T>` (из сырого указателя или через `MakeShared`) внутри объекта инициализируется `weak_this_`, после чего доступны:

* `SharedFromThis()`
* `WeakFromThis()`


## IntrusivePtr

Intrusive-подход: счётчик ссылок хранится внутри самого объекта (через `RefCounted`/`SimpleRefCounted`), а `IntrusivePtr` лишь вызывает `IncRef/DecRef`.

Компоненты:

* `SimpleCounter` — простой счётчик
* `RefCounted<Derived, Counter, Deleter>` и алиас `SimpleRefCounted<Derived>`
* `IntrusivePtr<T>`
* `MakeIntrusive<T>(args...)`

