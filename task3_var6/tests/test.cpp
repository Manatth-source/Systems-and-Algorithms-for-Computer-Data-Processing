#include "../src/heap.cpp"
#include "../src/scheduler.cpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>


// ====================================================================
//  Мини-фреймворк
// ====================================================================

static int g_checks = 0;
static int g_failed = 0;

#define CHECK(cond)                                                                      \
	do {                                                                                 \
		++g_checks;                                                                      \
		if (!(cond)) {                                                                   \
			++g_failed;                                                                  \
			std::cerr << "    FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n";  \
		}                                                                                \
	} while (0)

// Ожидаем исключение std::out_of_range.
#define CHECK_THROWS(expr)                                                               \
	do {                                                                                 \
		++g_checks;                                                                      \
		bool thrown_ = false;                                                            \
		try { (void)(expr); } catch (const std::out_of_range&) { thrown_ = true; }       \
		if (!thrown_) {                                                                  \
			++g_failed;                                                                  \
			std::cerr << "    FAIL " << __FILE__ << ":" << __LINE__                      \
			          << "  ожидалось исключение: " #expr "\n";                          \
		}                                                                                \
	} while (0)

struct TestCase { const char* name; void (*fn)(); };

static std::vector<TestCase>& registry()
{
	static std::vector<TestCase> r;
	return r;
}

struct Registrar
{
	Registrar(const char* name, void (*fn)()) { registry().push_back({ name, fn }); }
};

#define TEST(name)                                   \
	static void name();                              \
	static Registrar registrar_##name(#name, name);  \
	static void name()


// ====================================================================
//  Куча
// ====================================================================

using MinHeap = MyHeap::Heap<int, std::less<int>>;
using MaxHeap = MyHeap::Heap<int, std::greater<int>>;

// Вытащить всё из кучи в порядке приоритета. Заодно проверяет инвариант после КАЖДОГО pop.
template <class H>
static std::vector<int> drain(H& h)
{
	std::vector<int> out;
	while (!h.empty())
	{
		out.push_back(h.pop());
		CHECK(h.isValid());
	}
	return out;
}

static std::vector<int> randomVector(std::mt19937& rnd, std::size_t n, int range)
{
	std::vector<int> v(n);
	for (auto& x : v) x = static_cast<int>(rnd() % (2 * range + 1)) - range;
	return v;
}

// --- краевые случаи -------------------------------------------------

TEST(heap_empty)
{
	MinHeap h;
	CHECK(h.empty());
	CHECK(h.size() == 0);
	CHECK(h.isValid());
	CHECK_THROWS(h.pop());
	CHECK_THROWS(h.peek());
	CHECK_THROWS(h.replaceTop(1));
}

TEST(heap_single_element)
{
	MinHeap h;
	h.push(42);
	CHECK(!h.empty());
	CHECK(h.size() == 1);
	CHECK(h.peek() == 42);
	CHECK(h.isValid());

	CHECK(h.pop() == 42);
	CHECK(h.empty());
	CHECK(h.size() == 0);
	CHECK_THROWS(h.pop());       // после опустошения снова пусто
}

TEST(heap_peek_does_not_remove)
{
	MinHeap h;
	for (int x : { 5, 3, 8 }) h.push(x);
	CHECK(h.peek() == 3);
	CHECK(h.peek() == 3);        // повторный peek даёт то же
	CHECK(h.size() == 3);        // размер не изменился
}

TEST(heap_const_access)
{
	// Компилируется только если peek/size/empty помечены const.
	MinHeap h;
	h.push(2);
	h.push(1);
	const MinHeap& ch = h;
	CHECK(ch.size() == 2);
	CHECK(!ch.empty());
	CHECK(ch.peek() == 1);
}

TEST(heap_duplicates)
{
	MinHeap h;
	for (int x : { 7, 3, 7, 7, 3, 7, 7 }) h.push(x);
	CHECK(h.isValid());
	CHECK((drain(h) == std::vector<int>{ 3, 3, 7, 7, 7, 7, 7 }));

	MinHeap same;                // все элементы равны
	for (int i = 0; i < 50; ++i) same.push(9);
	CHECK(same.isValid());
	CHECK(drain(same).size() == 50);
}

// --- порядок: min / max ---------------------------------------------

TEST(heap_min_order_and_invariant)
{
	std::mt19937 rnd(1);
	auto data = randomVector(rnd, 1000, 50);       // много повторов
	MinHeap h;
	for (int x : data)
	{
		h.push(x);
		CHECK(h.isValid());                        // инвариант после каждой вставки
	}
	CHECK(h.size() == data.size());
	std::sort(data.begin(), data.end());
	CHECK(drain(h) == data);                       // pop выдаёт по возрастанию
}

TEST(heap_max_order_and_invariant)
{
	std::mt19937 rnd(2);
	auto data = randomVector(rnd, 1000, 50);
	MaxHeap h;
	for (int x : data)
	{
		h.push(x);
		CHECK(h.isValid());
	}
	std::sort(data.begin(), data.end(), std::greater<int>());
	CHECK(drain(h) == data);                       // pop выдаёт по убыванию
}

TEST(heap_default_is_max_heap)
{
	MyHeap::Heap<int> h;                           // компаратор по умолчанию
	for (int x : { 1, 5, 3 }) h.push(x);
	CHECK(h.peek() == 5);
}

TEST(heap_sorted_and_reverse_input)
{
	// Возрастающий и убывающий вход -- крайние случаи для всплытия и погружения.
	MinHeap asc, desc;
	for (int i = 0; i < 200; ++i) { asc.push(i); desc.push(199 - i); }
	CHECK(asc.isValid());
	CHECK(desc.isValid());
	auto a = drain(asc);
	auto d = drain(desc);
	CHECK(a == d);
	CHECK(std::is_sorted(a.begin(), a.end()));
}

// --- replaceTop -----------------------------------------------------

TEST(heap_replace_top_min)
{
	MinHeap h;
	for (int x : { 1, 3, 5, 8 }) h.push(x);

	CHECK(h.replaceTop(7) == 1);                   // вернул прежнюю вершину
	CHECK(h.size() == 4);                          // размер не изменился
	CHECK(h.isValid());
	CHECK(h.peek() == 3);                          // новая вершина -- наименьший из оставшихся
	CHECK((drain(h) == std::vector<int>{ 3, 5, 7, 8 }));
}

TEST(heap_replace_top_stays_on_top)
{
	MinHeap h;
	for (int x : { 4, 6, 9 }) h.push(x);
	CHECK(h.replaceTop(1) == 4);                   // новое значение меньше всех: остаётся наверху
	CHECK(h.peek() == 1);
	CHECK(h.isValid());
	CHECK(h.replaceTop(1) == 1);                   // равное значение
	CHECK(h.isValid());
}

TEST(heap_replace_top_single)
{
	MinHeap h;
	h.push(5);
	CHECK(h.replaceTop(9) == 5);
	CHECK(h.size() == 1);
	CHECK(h.peek() == 9);
}

TEST(heap_replace_top_max)
{
	MaxHeap h;
	for (int x : { 10, 7, 4, 1 }) h.push(x);
	CHECK(h.replaceTop(2) == 10);
	CHECK(h.isValid());
	CHECK((drain(h) == std::vector<int>{ 7, 4, 2, 1 }));
}

// --- heapify --------------------------------------------------------

TEST(heap_heapify_various_sizes)
{
	std::mt19937 rnd(3);
	for (std::size_t n : { 0u, 1u, 2u, 3u, 4u, 7u, 8u, 15u, 16u, 31u, 100u, 1000u })
	{
		auto data = randomVector(rnd, n, 30);

		MinHeap mn;
		mn.heapify(std::vector<int>(data));
		CHECK(mn.size() == n);
		CHECK(mn.isValid());                       // свойство кучи сразу после heapify
		auto sortedAsc = data;
		std::sort(sortedAsc.begin(), sortedAsc.end());
		CHECK(drain(mn) == sortedAsc);

		MaxHeap mx;
		mx.heapify(std::move(data));
		CHECK(mx.isValid());
		std::reverse(sortedAsc.begin(), sortedAsc.end());
		CHECK(drain(mx) == sortedAsc);
	}
}

TEST(heap_heapify_worst_best_equal)
{
	std::vector<int> desc, asc, equal(20, 4);
	for (int i = 15; i >= 1; --i) desc.push_back(i);   // худший случай для min-кучи
	for (int i = 1; i <= 15; ++i) asc.push_back(i);    // уже куча
	MinHeap a, b, c;
	a.heapify(std::move(desc));
	b.heapify(std::move(asc));
	c.heapify(std::move(equal));
	CHECK(a.isValid());
	CHECK(b.isValid());
	CHECK(c.isValid());
	std::vector<int> expected;
	for (int i = 1; i <= 15; ++i) expected.push_back(i);
	CHECK(drain(a) == expected);
	CHECK(drain(b) == expected);
	CHECK(drain(c).size() == 20);
}

TEST(heap_heapify_replaces_old_content)
{
	MinHeap h;
	for (int x : { 1, 2, 3 }) h.push(x);
	h.heapify(std::vector<int>{ 9, 8 });
	CHECK(h.size() == 2);
	CHECK((drain(h) == std::vector<int>{ 8, 9 }));
}

TEST(heap_heapify_then_push_pop)
{
	// После heapify куча остаётся рабочей.
	std::mt19937 rnd(4);
	auto data = randomVector(rnd, 200, 100);
	MinHeap h;
	h.heapify(std::vector<int>(data));
	for (int x : { -500, 500, 0 })
	{
		h.push(x);
		data.push_back(x);
		CHECK(h.isValid());
	}
	std::sort(data.begin(), data.end());
	CHECK(drain(h) == data);
}

// --- инвариант после серий случайных операций -----------------------

template <class Cmp>
static void randomOperations(unsigned seed)
{
	std::mt19937 rnd(seed);
	MyHeap::Heap<int, Cmp> h;
	std::multiset<int, Cmp> model;                 // эталонная модель для сверки

	for (int step = 0; step < 3000; ++step)
	{
		int op = static_cast<int>(rnd() % 10);
		if (op < 5 || model.empty())
		{
			int x = static_cast<int>(rnd() % 100);
			h.push(x);
			model.insert(x);
		}
		else if (op < 8)
		{
			int got = h.pop();
			CHECK(got == *model.begin());
			model.erase(model.begin());
		}
		else
		{
			int x = static_cast<int>(rnd() % 100);
			int got = h.replaceTop(x);
			CHECK(got == *model.begin());
			model.erase(model.begin());
			model.insert(x);
		}

		CHECK(h.size() == model.size());
		CHECK(h.isValid());                        // свойство кучи после КАЖДОЙ операции
		if (!model.empty()) CHECK(h.peek() == *model.begin());
	}
}

TEST(heap_random_operations_min)
{
	for (unsigned seed = 0; seed < 15; ++seed) randomOperations<std::less<int>>(seed);
}

TEST(heap_random_operations_max)
{
	for (unsigned seed = 0; seed < 15; ++seed) randomOperations<std::greater<int>>(seed);
}

// --- кастомный порядок и типы без копирования -----------------------

struct Job { int priority; int id; };
struct ByPriorityDesc
{
	bool operator()(const Job& a, const Job& b) const { return a.priority > b.priority; }
};

TEST(heap_custom_comparator)
{
	MyHeap::Heap<Job, ByPriorityDesc> h;           // наверху наибольший priority
	h.push({ 2, 100 });
	h.push({ 9, 101 });
	h.push({ 5, 102 });
	CHECK(h.peek().priority == 9);
	CHECK(h.pop().id == 101);
	CHECK(h.pop().id == 102);
	CHECK(h.pop().id == 100);
}

struct PtrLess
{
	bool operator()(const std::unique_ptr<int>& a, const std::unique_ptr<int>& b) const { return *a < *b; }
};

TEST(heap_move_only_type)
{
	// unique_ptr нельзя копировать: тест проходит только если куча везде перемещает.
	MyHeap::Heap<std::unique_ptr<int>, PtrLess> h;
	for (int x : { 5, 1, 4, 2, 3 }) h.push(std::make_unique<int>(x));
	for (int expected = 1; expected <= 5; ++expected)
	{
		auto p = h.pop();
		CHECK(*p == expected);
	}

	std::vector<std::unique_ptr<int>> v;
	for (int x : { 9, 7, 8 }) v.push_back(std::make_unique<int>(x));
	h.heapify(std::move(v));
	CHECK(*h.pop() == 7);
}


// ====================================================================
//  Планировщик
// ====================================================================

using Fired = std::vector<std::pair<Id, Time>>;

static std::string str(const Fired& f)
{
	std::ostringstream o;
	o << "[";
	for (std::size_t i = 0; i < f.size(); ++i)
		o << (i ? " " : "") << f[i].first << "@" << f[i].second;
	o << "]";
	return o.str();
}

static Fired advance(scheduler::Scheduler& s, Time to)
{
	Fired r;
	for (const auto& t : s.advance(to)) r.emplace_back(t.id, t.state.at);
	return r;
}

// CHECK_FIRED(actual, { {id, at}, ... })  -- сравнивает список сработавших
#define CHECK_FIRED(actual, ...)                                                          \
	do {                                                                                  \
		Fired a_ = (actual);                                                              \
		Fired e_ = __VA_ARGS__;                                                           \
		++g_checks;                                                                       \
		if (a_ != e_) {                                                                   \
			++g_failed;                                                                   \
			std::cerr << "    FAIL " << __FILE__ << ":" << __LINE__                       \
			          << "  ожидалось " << str(e_) << ", получено " << str(a_) << "\n";   \
		}                                                                                 \
	} while (0)

// Простая эталонная модель: только словарь, фильтрация и сортировка при advance.
class ModelScheduler
{
public:
	void set(Id id, Time at) { active_[id] = { at, ++order_ }; }
	void cancel(Id id) { active_.erase(id); }
	Fired advance(Time to)
	{
		std::vector<std::tuple<Time, int, Id>> due;
		for (const auto& kv : active_)
			if (kv.second.first <= to)
				due.emplace_back(kv.second.first, kv.second.second, kv.first);
		std::sort(due.begin(), due.end());         // по времени, при равенстве по порядку назначений
		Fired r;
		for (const auto& d : due)
		{
			active_.erase(std::get<2>(d));
			r.emplace_back(std::get<2>(d), std::get<0>(d));
		}
		return r;
	}
private:
	std::unordered_map<Id, std::pair<Time, int>> active_;
	int order_ = 0;
};

// --- базовое поведение ------------------------------------------------

TEST(sched_empty_advance)
{
	scheduler::Scheduler s;
	CHECK_FIRED(advance(s, 0), {});
	CHECK_FIRED(advance(s, 100), {});
}

TEST(sched_set_then_advance_fires_once)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	CHECK_FIRED(advance(s, 5), { {1, 5} });
	CHECK_FIRED(advance(s, 5), {});                // повторный advance на то же время
	CHECK_FIRED(advance(s, 100), {});              // сработавший не срабатывает снова
}

TEST(sched_boundary)
{
	scheduler::Scheduler s;
	s.set(1, 10);
	s.set(2, 11);
	CHECK_FIRED(advance(s, 9), {});                // at > to: не срабатывает
	CHECK_FIRED(advance(s, 10), { {1, 10} });      // at == to: срабатывает (граница включается)
	CHECK_FIRED(advance(s, 11), { {2, 11} });
}

TEST(sched_time_zero_and_set_at_current_time)
{
	scheduler::Scheduler s;
	s.set(1, 0);                                   // начальное время 0
	CHECK_FIRED(advance(s, 0), { {1, 0} });

	advance(s, 10);
	s.set(2, 10);                                  // at == текущее время
	CHECK_FIRED(advance(s, 10), { {2, 10} });      // срабатывает на следующем advance
}

TEST(sched_advance_without_events)
{
	scheduler::Scheduler s;
	s.set(1, 100);
	CHECK_FIRED(advance(s, 10), {});
	CHECK_FIRED(advance(s, 50), {});
	CHECK(s.activeCount() == 1);                   // таймер никуда не делся
	CHECK_FIRED(advance(s, 100), { {1, 100} });
}

TEST(sched_order_by_time)
{
	scheduler::Scheduler s;
	s.set(3, 30);
	s.set(1, 10);
	s.set(2, 20);
	CHECK_FIRED(advance(s, 100), { {1, 10}, {2, 20}, {3, 30} });
}

TEST(sched_new_timers_after_time_moved)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	s.set(2, 15);
	CHECK_FIRED(advance(s, 10), { {1, 5} });
	s.set(3, 12);
	CHECK_FIRED(advance(s, 20), { {3, 12}, {2, 15} });
}

// --- равные времена ---------------------------------------------------

TEST(sched_equal_times_assignment_order)
{
	scheduler::Scheduler s;
	s.set(2, 5);                                   // id намеренно не по возрастанию:
	s.set(1, 5);                                   // порядок определяется назначением, а не id
	s.set(3, 5);
	CHECK_FIRED(advance(s, 5), { {2, 5}, {1, 5}, {3, 5} });
}

TEST(sched_reassign_same_time_goes_last)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	s.set(2, 5);
	s.set(1, 5);                                   // новое назначение: теперь 1 после 2
	CHECK_FIRED(advance(s, 5), { {2, 5}, {1, 5} });
}

// --- перенос ----------------------------------------------------------

TEST(sched_move_later)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	s.set(1, 20);
	CHECK_FIRED(advance(s, 10), {});               // старое назначение на 5 не срабатывает
	CHECK_FIRED(advance(s, 20), { {1, 20} });
	CHECK_FIRED(advance(s, 100), {});
}

TEST(sched_move_earlier)
{
	scheduler::Scheduler s;
	s.set(1, 50);
	s.set(1, 7);
	CHECK_FIRED(advance(s, 10), { {1, 7} });
	CHECK_FIRED(advance(s, 100), {});              // старое назначение на 50 -- устаревшее
}

TEST(sched_stale_earlier_entry_must_not_fire)
{
	// Именно здесь ломается реализация без проверки номера назначения:
	// в куче лежит запись (id 1, at 3), а актуальное время -- 10.
	scheduler::Scheduler s;
	s.set(1, 3);
	s.set(1, 10);
	CHECK_FIRED(advance(s, 5), {});
	CHECK_FIRED(advance(s, 10), { {1, 10} });
}

TEST(sched_move_back_to_old_time)
{
	scheduler::Scheduler s;
	s.set(1, 10);
	s.set(2, 10);
	s.set(1, 20);
	s.set(1, 10);                                  // вернули на прежнее время: это НОВОЕ назначение
	CHECK_FIRED(advance(s, 10), { {2, 10}, {1, 10} });
	CHECK_FIRED(advance(s, 30), {});               // запись на 20 тоже устаревшая
}

TEST(sched_move_many_times)
{
	scheduler::Scheduler s;
	for (int i = 0; i < 100; ++i) s.set(1, 1000 - i);   // последнее назначение: 901
	CHECK_FIRED(advance(s, 900), {});
	CHECK_FIRED(advance(s, 901), { {1, 901} });
	CHECK_FIRED(advance(s, 5000), {});
}

// --- отмена -----------------------------------------------------------

TEST(sched_cancel)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	s.cancel(1);
	CHECK(s.activeCount() == 0);
	CHECK_FIRED(advance(s, 10), {});
}

TEST(sched_cancel_missing_is_ignored)
{
	scheduler::Scheduler s;
	s.cancel(42);                                  // на пустом планировщике
	s.set(1, 5);
	s.cancel(42);                                  // несуществующий ID рядом с существующим
	CHECK_FIRED(advance(s, 5), { {1, 5} });
}

TEST(sched_cancel_twice_and_after_fire)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	s.cancel(1);
	s.cancel(1);                                   // повторная отмена
	CHECK_FIRED(advance(s, 10), {});

	s.set(2, 12);
	CHECK_FIRED(advance(s, 12), { {2, 12} });
	s.cancel(2);                                   // отмена уже сработавшего
	s.set(2, 20);
	CHECK_FIRED(advance(s, 20), { {2, 20} });      // и это не помешало новому назначению
}

TEST(sched_cancel_one_of_many)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	s.set(2, 5);
	s.set(3, 5);
	s.cancel(2);
	CHECK_FIRED(advance(s, 5), { {1, 5}, {3, 5} });
}

// --- повторное использование ID ----------------------------------------

TEST(sched_reuse_id_after_fire)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	CHECK_FIRED(advance(s, 5), { {1, 5} });
	s.set(1, 8);
	CHECK_FIRED(advance(s, 8), { {1, 8} });
}

TEST(sched_reuse_id_after_cancel)
{
	// Старая отменённая запись (at 5) остаётся в куче и не должна срабатывать
	// из-за того, что новое назначение получило тот же id.
	scheduler::Scheduler s;
	s.set(1, 5);
	s.cancel(1);
	s.set(1, 9);
	CHECK_FIRED(advance(s, 7), {});
	CHECK_FIRED(advance(s, 9), { {1, 9} });
}

TEST(sched_cancel_then_set_same_time)
{
	scheduler::Scheduler s;
	s.set(1, 5);
	s.cancel(1);
	s.set(1, 5);                                   // в куче две записи с одинаковым (id, at)
	CHECK_FIRED(advance(s, 5), { {1, 5} });        // срабатывает ровно один раз
	CHECK_FIRED(advance(s, 10), {});
}

// --- размер кучи с устаревшими назначениями -----------------------------

TEST(sched_heap_size_counts_stale_after_moves)
{
	scheduler::Scheduler s;
	for (int i = 0; i < 100; ++i) s.set(1, 1000 + i);
	CHECK(s.activeCount() == 1);
	CHECK(s.heapSize() == 100);                    // 1 актуальная и 99 устаревших

	CHECK_FIRED(advance(s, 500), {});
	CHECK(s.heapSize() == 100);                    // до устаревших ещё не дошли

	CHECK_FIRED(advance(s, 5000), { {1, 1099} });
	CHECK(s.heapSize() == 0);                      // мусор вычищен при извлечении
}

TEST(sched_heap_size_counts_stale_after_cancel)
{
	scheduler::Scheduler s;
	for (int i = 0; i < 50; ++i) { s.set(i, 100 + i); s.cancel(i); }
	CHECK(s.activeCount() == 0);
	CHECK(s.heapSize() == 50);                     // отмена не удаляет запись из кучи
	CHECK_FIRED(advance(s, 1000), {});
	CHECK(s.heapSize() == 0);
}

// --- масштаб и сверка с эталонной моделью --------------------------------

TEST(sched_many_timers_sorted_with_ties)
{
	std::mt19937 rnd(5);
	scheduler::Scheduler s;
	std::vector<std::pair<Time, Id>> expected;     // (at, id); id растёт вместе с порядком назначений
	for (Id id = 0; id < 5000; ++id)
	{
		Time at = static_cast<Time>(rnd() % 1000);   // много одинаковых времён
		s.set(id, at);
		expected.emplace_back(at, id);
	}
	std::sort(expected.begin(), expected.end());     // при равных at меньший id назначен раньше

	Fired exp;
	for (const auto& e : expected) exp.emplace_back(e.second, e.first);
	CHECK(advance(s, 999) == exp);
	CHECK(s.activeCount() == 0);
	CHECK(s.heapSize() == 0);
}

TEST(sched_matches_model_on_random_commands)
{
	for (unsigned seed = 0; seed < 40; ++seed)
	{
		std::mt19937 rnd(seed);
		scheduler::Scheduler s;
		ModelScheduler model;
		Time now = 0;

		for (int step = 0; step < 3000; ++step)
		{
			Id id = static_cast<Id>(rnd() % 8);        // мало ID: много переносов, отмен и повторов
			int op = static_cast<int>(rnd() % 10);
			if (op < 5)
			{
				Time at = now + static_cast<Time>(rnd() % 15);   // at не меньше текущего времени
				s.set(id, at);
				model.set(id, at);
			}
			else if (op < 7)
			{
				s.cancel(id);
				model.cancel(id);
			}
			else
			{
				now += static_cast<Time>(rnd() % 6);             // to не убывает, равное допустимо
				Fired got = advance(s, now);
				Fired exp = model.advance(now);
				++g_checks;
				if (got != exp)
				{
					++g_failed;
					std::cerr << "    FAIL seed " << seed << " step " << step << ": ожидалось "
						<< str(exp) << ", получено " << str(got) << "\n";
					return;
				}
			}
			CHECK(s.heapSize() >= s.activeCount());          // в куче есть все актуальные назначения
		}

		now += 1000;                                         // в конце забираем всё оставшееся
		CHECK(advance(s, now) == model.advance(now));
	}
}


// ====================================================================

// Запускает все тесты. Возвращает 0, если всё прошло.
int runAllTests()
{
	int failedTests = 0;

	for (const auto& t : registry())
	{
		int before = g_failed;
		try
		{
			t.fn();
		}
		catch (const std::exception& e)
		{
			++g_failed;
			std::cerr << "    FAIL неожиданное исключение: " << e.what() << "\n";
		}

		bool ok = (g_failed == before);
		if (!ok) ++failedTests;
		std::cout << (ok ? "[ OK ]  " : "[FAIL]  ") << t.name << "\n";
	}

	std::cout << "\nтестов: " << registry().size()
		<< ", проверок: " << g_checks
		<< ", упавших тестов: " << failedTests << "\n";
	return failedTests ? 1 : 0;
}
