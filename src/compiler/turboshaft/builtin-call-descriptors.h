// Copyright 2023 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_COMPILER_TURBOSHAFT_BUILTIN_CALL_DESCRIPTORS_H_
#define V8_COMPILER_TURBOSHAFT_BUILTIN_CALL_DESCRIPTORS_H_

#include "src/codegen/callable.h"
#include "src/codegen/interface-descriptors.h"
#include "src/compiler/frame.h"
#include "src/compiler/turboshaft/operations.h"
#include "src/compiler/turboshaft/representations.h"

namespace v8::internal::compiler::turboshaft {

struct BuiltinCallDescriptor {
 private:
  template <typename Derived>
  struct Descriptor {
    static const TSCallDescriptor* Create(Isolate* isolate, Zone* zone) {
      Callable callable = Builtins::CallableFor(isolate, Derived::Function);
      auto descriptor = Linkage::GetStubCallDescriptor(
          zone, callable.descriptor(),
          callable.descriptor().GetStackParameterCount(),
          Derived::NeedsFrameState ? CallDescriptor::kNeedsFrameState
                                   : CallDescriptor::kNoFlags,
          Derived::Properties);
#ifdef DEBUG
      Derived::Verify(descriptor);
#endif  // DEBUG
      return TSCallDescriptor::Create(descriptor, zone);
    }

#ifdef DEBUG
    static void Verify(const CallDescriptor* desc) {
      using result_t = typename Derived::result_t;
      using arguments_t = typename Derived::arguments_t;
      if constexpr (std::is_same_v<result_t, void>) {
        DCHECK_EQ(desc->ReturnCount(), 0);
      } else {
        DCHECK_EQ(desc->ReturnCount(), 1);
        DCHECK(result_t::rep_type::allows_representation(
            RegisterRepresentation::FromMachineRepresentation(
                desc->GetReturnType(0).representation())));
      }
      DCHECK_EQ(desc->NeedsFrameState(), Derived::NeedsFrameState);
      DCHECK_EQ(desc->properties(), Derived::Properties);
      constexpr int context_argument_count = 1;  // Context or NoContextConstant
      DCHECK_EQ(desc->ParameterCount(),
                std::tuple_size_v<arguments_t> + context_argument_count);
      DCHECK(VerifyArguments<arguments_t>(desc));
    }

    template <typename Arguments>
    static bool VerifyArguments(const CallDescriptor* desc) {
      return VerifyArgumentsImpl<Arguments>(
          desc, std::make_index_sequence<std::tuple_size_v<Arguments>>());
    }

   private:
    template <typename Arguments, size_t... Indices>
    static bool VerifyArgumentsImpl(const CallDescriptor* desc,
                                    std::index_sequence<Indices...>) {
      return (std::tuple_element_t<Indices, Arguments>::rep_type::
                  allows_representation(
                      RegisterRepresentation::FromMachineRepresentation(
                          desc->GetParameterType(Indices).representation())) &&
              ...);
    }
#endif  // DEBUG
  };

 public:
  struct StringFromCodePointAt : public Descriptor<StringFromCodePointAt> {
    static constexpr auto Function = Builtin::kStringFromCodePointAt;
    using arguments_t = std::tuple<V<Tagged>, V<WordPtr>>;
    using result_t = V<Tagged>;

    static constexpr bool NeedsFrameState = false;
    static constexpr bool NeedsContext = false;
    static constexpr Operator::Properties Properties = Operator::kEliminatable;
  };

  struct StringIndexOf : public Descriptor<StringIndexOf> {
    static constexpr auto Function = Builtin::kStringIndexOf;
    using arguments_t = std::tuple<V<Tagged>, V<Tagged>, V<Tagged>>;
    using result_t = V<Tagged>;

    static constexpr bool NeedsFrameState = false;
    static constexpr bool NeedsContext = false;
    static constexpr Operator::Properties Properties = Operator::kEliminatable;
  };

  struct StringSubstring : public Descriptor<StringSubstring> {
    static constexpr auto Function = Builtin::kStringSubstring;
    using arguments_t = std::tuple<V<Tagged>, V<WordPtr>, V<WordPtr>>;
    using result_t = V<Tagged>;

    static constexpr bool NeedsFrameState = false;
    static constexpr bool NeedsContext = false;
    static constexpr Operator::Properties Properties = Operator::kEliminatable;
  };

#ifdef V8_INTL_SUPPORT
  struct StringToLowerCaseIntl : public Descriptor<StringToLowerCaseIntl> {
    static constexpr auto Function = Builtin::kStringToLowerCaseIntl;
    using arguments_t = std::tuple<V<Tagged>>;
    using result_t = V<Tagged>;

    static constexpr bool NeedsFrameState = false;
    static constexpr bool NeedsContext = false;
    static constexpr Operator::Properties Properties =
        Operator::kNoDeopt | Operator::kNoThrow;
  };
#endif  // V8_INTL_SUPPORT
};

}  // namespace v8::internal::compiler::turboshaft

#endif  // V8_COMPILER_TURBOSHAFT_BUILTIN_CALL_DESCRIPTORS_H_
