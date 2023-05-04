#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <tuple>
namespace Cosmos {
namespace details {
///@note tuple 参数的索引序列的定义
template <int...> struct IndexTuple {};

/// @note 继承方式，开始展开参数包
template <int N, int... Indexes>
struct MakeIndexes : MakeIndexes<N - 1, N - 1, Indexes...> {};

/// @note 模板特化，终止展开参数包的条件
template <int... indexes> struct MakeIndexes<0, indexes...> {
  typedef IndexTuple<indexes...> type;
};

/// @note 类型重定义（用两个 tuple 的元素类型，重新定义一个新 pair 类型）
/// key 为消息，value 为处理函数
template <std::size_t N, typename T1, typename T2>
using pair_type = std::pair<typename std::tuple_element<N, T1>::type,
                            typename std::tuple_element<N, T2>::type>;

/// @note 同上，返回新的 pair
template <std::size_t N, typename T1, typename T2>
pair_type<N, T1, T2> pair(const T1 &tup1, const T2 &tup2) {
  return std::make_pair(std::get<N>(tup1), std::get<N>(tup2));
}

/// @note 将不同 index 的 pair （每个都是由两个 tuple 元素组成）构造成新的 tuple
template <int... Indexes, typename T1, typename T2>
auto pairs_helper(IndexTuple<Indexes...>, const T1 &tup1, const T2 &tup2)
    -> decltype(std::make_tuple(pair<Indexes>(tup1, tup2)...)) {
  return std::make_tuple(pair<Indexes>(tup1, tup2)...);
}

} // namespace details

/// @note 将两个 tuple 合并为一个新 tuple
template <typename Tuple1, typename Tuple2>
auto Zip(Tuple1 tup1, Tuple2 tup2) -> decltype(details::pairs_helper(
    typename details::MakeIndexes<std::tuple_size<Tuple1>::value>::type(), tup1,
    tup2)) {
  /// @note 两个 tuple 的 size 需要相同
  static_assert(std::tuple_size<Tuple1>::value ==
                    std::tuple_size<Tuple2>::value,
                "tuples should be the same size.");
  /// @note 将不同 index 的 pair 构造为新的 tuple
  return details::pairs_helper(
      typename details::MakeIndexes<std::tuple_size<Tuple1>::value>::type(),
      tup1, tup2);
}

template <typename F, typename... Args>
auto Apply(F &&f, Args &&...args) -> decltype(f(args...)) {
  /// @note 先完美转发，再调用 f(args...)
  return std::forward<F>(f)(std::forward<Args>(args)...);
}
} // namespace Cosmos

#endif // __MESSAGE_HPP__