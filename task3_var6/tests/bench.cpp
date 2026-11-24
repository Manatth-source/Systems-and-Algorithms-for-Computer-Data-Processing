// Замеры: куча с ленивым удалением (scheduler::Scheduler) против эталона
// (словарь активных + фильтрация и сортировка при ADVANCE).
//
// Запуск из main.cpp:   --bench [maxN] [csv]
// Замеряется только время самих вызовов set / cancel / advance в тиках процессора (rdtsc).
// Генерация сценария, проверка корректности и печать -- вне замера.
#include "../src/heap.cpp"
#include "../src/scheduler.cpp"
#include <algorithm>
#include <cstdint>
#include <format>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <version>

#if defined(__cpp_lib_print)
#include <print>            // C++23: std::print
#endif

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif


// ---------------- вывод ----------------
// C++23: std::print, если стандартная библиотека его поддерживает; иначе cout + std::format.
template <class... Args>
static void out(std::format_string<Args...> fmt, Args&&... args)
{
#if defined(__cpp_lib_print)
	std::print(fmt, std::forward<Args>(args)...);
#else
	std::cout << std::format(fmt, std::forward<Args>(args)...);
#endif
}


// ---------------- эталон ----------------
class RefScheduler
{
public:
	void set(Id id, Time at) { active_[id] = { at, ++order_ }; }
	void cancel(Id id) { active_.erase(id); }

	std::vector<timer::Timer> advance(Time to)
	{
		std::vector<timer::Timer> due;

		for (const auto& kv : active_)                          // фильтрация
			if (kv.second.first <= to)
				due.push_back({ kv.first, { kv.second.first, kv.second.second } });

		std::sort(due.begin(), due.end());                      // сортировка

		for (const auto& d : due) active_.erase(d.id);
		return due;
	}
private:
	std::unordered_map<Id, std::pair<Time, int>> active_;
	int order_ = 0;
};


// ---------------- сценарии ----------------
enum Type { SET, CANCEL, ADVANCE };
struct Cmd { Type type; Id id; Time arg; };

struct Params
{
	const char* name;
	const char* description;
	int pSet, pCancel;      // проценты; остальное -- ADVANCE
	double idsFrac;         // число различных ID = N * idsFrac
	double window;          // at = now + rnd % (window * N)
	int step;               // to = now + rnd % step
};

static std::vector<Cmd> makeScenario(std::size_t n, const Params& p, unsigned seed)
{
	std::mt19937 rnd(seed);
	std::vector<Cmd> v;
	v.reserve(n);
	Time now = 0;
	const unsigned ids = std::max<unsigned>(1, static_cast<unsigned>(n * p.idsFrac));
	const Time window = std::max<Time>(1, static_cast<Time>(p.window * n));
	for (std::size_t i = 0; i < n; ++i)
	{
		int r = static_cast<int>(rnd() % 100);
		Id id = static_cast<Id>(rnd() % ids);
		if (r < p.pSet)
			v.push_back({ SET, id, now + static_cast<Time>(rnd() % window) });
		else if (r < p.pSet + p.pCancel)
			v.push_back({ CANCEL, id, 0 });
		else
		{
			now += static_cast<Time>(rnd() % p.step);
			v.push_back({ ADVANCE, 0, now });
		}
	}
	return v;
}


// ---------------- замер ----------------
static uint64_t g_overhead = 0;     // стоимость самой пары rdtsc, вычитается из каждого замера
// volatile-переменная: компилятор обязан реально писать в неё,
// поэтому не может выкинуть вычисление результата как "неиспользуемое"
static volatile long long g_sink = 0;

struct Ticks { double set = 0, cancel = 0, advance = 0; };   // суммарные тики по методам

template <class S>
static Ticks runOnce(const std::vector<Cmd>& cmds)
{
	S s;
	uint64_t tSet = 0, tCancel = 0, tAdv = 0, sink = 0;
	for (const auto& c : cmds)
	{
		switch (c.type)
		{
		case SET: {
			uint64_t a = __rdtsc();
			s.set(c.id, c.arg);
			uint64_t b = __rdtsc();
			tSet += b - a - g_overhead;
			break;
		}
		case CANCEL: {
			uint64_t a = __rdtsc();
			s.cancel(c.id);
			uint64_t b = __rdtsc();
			tCancel += b - a - g_overhead;
			break;
		}
		case ADVANCE: {
			uint64_t a = __rdtsc();
			auto fired = s.advance(c.arg);
			uint64_t b = __rdtsc();
			tAdv += b - a - g_overhead;
			for (const auto& t : fired)
				sink += static_cast<uint64_t>(t.id) * 31 + static_cast<uint64_t>(t.state.at);
			break;
		}
		}
	}
	g_sink = g_sink + static_cast<long long>(sink);
	return { static_cast<double>(tSet), static_cast<double>(tCancel), static_cast<double>(tAdv) };
}

static double median(std::vector<double> v)
{
	std::sort(v.begin(), v.end());
	return v[v.size() / 2];
}

// Повторы, медиана суммарных тиков по каждому методу.
template <class S>
static Ticks measure(const std::vector<Cmd>& cmds, int reps)
{
	std::vector<double> vs, vc, va;
	for (int i = 0; i < reps; ++i)
	{
		Ticks t = runOnce<S>(cmds);
		vs.push_back(t.set);
		vc.push_back(t.cancel);
		va.push_back(t.advance);
	}
	return { median(vs), median(vc), median(va) };
}

// Обе реализации должны выдавать одинаковые списки сработавших (вне замера).
static bool sameOutput(const std::vector<Cmd>& cmds)
{
	scheduler::Scheduler a;
	RefScheduler b;
	for (const auto& c : cmds)
	{
		if (c.type == SET) { a.set(c.id, c.arg); b.set(c.id, c.arg); }
		else if (c.type == CANCEL) { a.cancel(c.id); b.cancel(c.id); }
		else
		{
			auto x = a.advance(c.arg);
			auto y = b.advance(c.arg);
			if (x.size() != y.size()) return false;
			for (std::size_t i = 0; i < x.size(); ++i)
				if (x[i].id != y[i].id || x[i].state.at != y[i].state.at) return false;
		}
	}
	return true;
}


// ---------------- вывод результатов ----------------
static void printMethodRow(const char* method, std::size_t calls, double heapTicks, double refTicks)
{
	double ratio = heapTicks > 0 ? refTicks / heapTicks : 0;
	out("  {:<8}{:>10}{:>16}{:>16}{:>12.2f}\n", method, calls,
		static_cast<uint64_t>(heapTicks), static_cast<uint64_t>(refTicks), ratio);
}

static void printCsvRow(const char* scenario, std::size_t n, const char* impl, const Ticks& t,
	std::size_t nSet, std::size_t nCancel, std::size_t nAdv)
{
	out("{},{},{},{},{},{},{},{},{}\n", scenario, n, impl, nSet, nCancel, nAdv,
		static_cast<uint64_t>(t.set), static_cast<uint64_t>(t.cancel), static_cast<uint64_t>(t.advance));
}


// Вызывается из main.cpp по ключу --bench [maxN] [csv]
int runBench(std::size_t maxN, bool csv)
{
	const int REPS = 7;
	const std::size_t MIN_N = 1024;

	// накладные расходы самого замера (пара rdtsc подряд), в тиках
	{
		uint64_t best = ~0ull;
		for (int i = 0; i < 100000; ++i) { uint64_t a = __rdtsc(); uint64_t b = __rdtsc(); best = std::min(best, b - a); }
		g_overhead = best;
	}

	const Params scenarios[] = {
		{ "mixed",      "типичный поток: 55% SET, 20% CANCEL, 25% ADVANCE",
		  55, 20, 0.50, 2.0, 10 },
		{ "move_heavy", "много переносов и мусора в куче: 80% SET, 5% CANCEL, 15% ADVANCE, мало различных ID",
		  80, 5, 0.05, 4.0, 10 },
	};

	if (csv)
		out("scenario,N,impl,n_set,n_cancel,n_advance,set_total_ticks,cancel_total_ticks,advance_total_ticks\n");
	else
	{
		out("================================================================\n");
		out(" ЗАМЕРЫ: куча с ленивым удалением  против  эталона\n");
		out("================================================================\n");
		out(" Единица измерения : тики процессора (rdtsc), СУММАРНО на все вызовы метода в сценарии\n");
		out(" Повторов          : {}, берётся медиана\n", REPS);
		out(" Накладные расходы : {} тиков на замер, вычтены из результата\n", g_overhead);
		out(" Размеры N         : от {} до {} команд, шаг x2\n", MIN_N, maxN);
		out(" Столбцы           : вызовов | куча | эталон | эталон/куча\n");
		out("                     эталон/куча > 1 - куча быстрее, < 1 - эталон быстрее\n");
		out(" Результаты обеих реализаций перед замером сверяются между собой\n");
		out("================================================================\n");
	}

	for (const auto& sc : scenarios)
	{
		if (!csv)
		{
			out("\n################################################################\n");
			out(" СЦЕНАРИЙ: {}\n", sc.name);
			out(" {}\n", sc.description);
			out("################################################################\n");
		}

		for (std::size_t n = MIN_N; n <= maxN; n *= 2)
		{
			auto cmds = makeScenario(n, sc, 12345);

			std::size_t nSet = 0, nCancel = 0, nAdv = 0;
			for (const auto& c : cmds)
				(c.type == SET ? nSet : c.type == CANCEL ? nCancel : nAdv)++;

			if (!sameOutput(cmds))
			{
				out("ОШИБКА: результаты кучи и эталона не совпали (сценарий {}, N = {})\n", sc.name, n);
				return 1;
			}

			Ticks heap = measure<scheduler::Scheduler>(cmds, REPS);
			Ticks ref = measure<RefScheduler>(cmds, REPS);

			if (csv)
			{
				printCsvRow(sc.name, n, "heap", heap, nSet, nCancel, nAdv);
				printCsvRow(sc.name, n, "ref", ref, nSet, nCancel, nAdv);
				continue;
			}

			out("\n--- N = {} команд  (SET: {}, CANCEL: {}, ADVANCE: {}) ---\n", n, nSet, nCancel, nAdv);
			out("  метод     вызовов            куча          эталон  эталон/куча\n");
			printMethodRow("set", nSet, heap.set, ref.set);
			printMethodRow("cancel", nCancel, heap.cancel, ref.cancel);
			printMethodRow("advance", nAdv, heap.advance, ref.advance);
		}
	}

	if (!csv)
	{
		const long long sinkValue = g_sink;
		out("\nКонтрольная сумма (чтобы оптимизатор не выбросил вычисления): {}\n", sinkValue);
	}
	return 0;
}