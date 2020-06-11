#pragma once

namespace mxx::type_traits {

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_bang_handler, handle_bang, long());

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_int_handler,
                                     handle_int,
                                     long(),
                                     long());

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_float_handler,
                                     handle_float,
                                     double(),
                                     long());

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_list_handler,
                                     handle_list,
                                     std::declval<::mxx::atom::vector>(),
                                     long());

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_raw_list_handler,
                                     handle_raw_list,
                                     std::declval<::mxx::atom::span>(),
                                     long());

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_dsp_handler,
                                     process,
                                     std::declval<double**>(),
                                     std::declval<double**>(),
                                     long(),
                                     long(),
                                     long());

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_prepare_function,
                                     prepare,
                                     double(),
                                     long());

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_construct_function,
                                     construct,
                                     std::declval<::mxx::atom::vector>());

    MXX_CREATE_ADVANCED_MEMBER_CHECK(has_input_changed_function,
                                     inputchanged,
                                     long());

    template <typename user_class>
    std::integral_constant<bool, has_dsp_handler_impl<user_class>::value>
        is_dsp_class;

}    // namespace mxx::type_traits