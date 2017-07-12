//
// Created by svakhreev on 02.03.17.
//

#ifndef CAMERAMANAGERCORE_DETECTORS_HPP
#define CAMERAMANAGERCORE_DETECTORS_HPP

#include <type_traits>

namespace cameramanagercore::model::detail
{

template < typename Tleft, typename Tright, template <typename, typename> class Detector >
struct not_appliable {
    struct success {};
    template <class Tl, class Tr> static Detector<Tl, Tr> detector_impl(long) noexcept;
    template <class Tl, class Tr> static success detector_impl(int) noexcept;

    static constexpr bool value = std::is_same_v <
        decltype(detector_impl<Tleft, Tright>(1L)),
        success >;
};

template <class T1, class T2> using comp_eq_detector     = decltype(std::declval<T1>() == std::declval<T2>());
template <class T1, class T2> using comp_ne_detector     = decltype(std::declval<T1>() != std::declval<T2>());
template <class T1, class T2> using comp_lt_detector     = decltype(std::declval<T1>() <  std::declval<T2>());
template <class T1, class T2> using comp_le_detector     = decltype(std::declval<T1>() <= std::declval<T2>());
template <class T1, class T2> using comp_gt_detector     = decltype(std::declval<T1>() >  std::declval<T2>());
template <class T1, class T2> using comp_ge_detector     = decltype(std::declval<T1>() >= std::declval<T2>());
template <class S,  class T>  using ostreamable_detector  = decltype(std::declval<S>()  << std::declval<T>());
template <class S,  class T>  using istreamable_detector  = decltype(std::declval<S>()  >> std::declval<T>());

template <template <typename, typename> class Detector, typename T>
using enable_flat_not_comp_base_t = std::enable_if_t <
    not_appliable < T const&, T const&, Detector >::value,
    bool
>;

template <class T> using enable_flat_not_eq_comp_t = enable_flat_not_comp_base_t<comp_eq_detector, T>;
template <class T> using enable_flat_not_ne_comp_t = enable_flat_not_comp_base_t<comp_ne_detector, T>;
template <class T> using enable_flat_not_lt_comp_t = enable_flat_not_comp_base_t<comp_lt_detector, T>;
template <class T> using enable_flat_not_le_comp_t = enable_flat_not_comp_base_t<comp_le_detector, T>;
template <class T> using enable_flat_not_gt_comp_t = enable_flat_not_comp_base_t<comp_gt_detector, T>;
template <class T> using enable_flat_not_ge_comp_t = enable_flat_not_comp_base_t<comp_ge_detector, T>;

template <typename Stream, typename Type>
using enable_flat_not_ostreamable_t = typename std::enable_if_t <
    not_appliable<Stream&, Type const&, ostreamable_detector>::value,
    Stream&
>;

template <typename Stream, typename Type>
using enable_flat_not_istreamable_t = typename std::enable_if_t<
    not_appliable<Stream&, Type&, istreamable_detector>::value,
    Stream&
>;

} // namespace boost::pfr::detail

#endif //CAMERAMANAGERCORE_DETECTORS_HPP
