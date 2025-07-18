// Copyright 2025 The XLS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

fn and_reduce<N: u32>(x: uN[N]) -> u1;

fn array_rev<T: type, N: u32>(x: T[N]) -> T[N];

fn array_size<T: type, N: u32>(x: T[N]) -> u32;

fn array_slice<T: type, M: u32, N: u32, P: u32>(xs: T[M], start: uN[N], want: T[P]) -> T[P];

fn assert_eq<T: type>(x: T, y: T) -> ();

fn assert_lt<S: bool, N: u32>(x: xN[S][N], y: xN[S][N]) -> ();

fn assert!<N: u32>(predicate: bool, label: u8[N]) -> ();

fn bit_count<T: type>() -> u32;

fn bit_slice_update<N: u32, U: u32, V: u32>(x: uN[N], y: uN[U], z: uN[V]) -> uN[N];

fn checked_cast<DEST: type, SRC: type>(x: SRC) -> DEST;

// This must be before ceillog2 so that the parser uses the BuiltinNameDef for ceillog2,
// and not the subsequent stub.
fn encode<N: u32, M: u32={ceillog2(N)}>(x: uN[N]) -> uN[M];

fn ceillog2<N: u32>(x: uN[N]) -> uN[N];

fn clz<N: u32>(x: uN[N]) -> uN[N];

fn cover!<N: u32>(name: u8[N], condition: u1) -> ();

fn ctz<N: u32>(x: uN[N]) -> uN[N];

// The type of `f` must be fn f(x: T) -> U
fn map<F: type, U: type, T: type, N: u32>(arr: T[N], f: F) -> U[N];

// T must be an unsigned type.
fn decode<T: type, N: u32>(x: uN[N]) -> T;

fn element_count<T: type>() -> u32;

fn enumerate<T: type, N: u32>(x: T[N]) -> (u32, T)[N];

fn fail!<N: u32, T: type> (label: u8[N], fallback_value: T) -> T;

fn gate!<T: type>(x: u1, y: T) -> T;

// `join` can take zero or more `token` arguments; the varargs are handled by the type checker.
fn join(t: token) -> token;

fn one_hot<N: u32, M: u32={N + 1}>(x: uN[N], y: u1) -> uN[M];

fn one_hot_sel<N: u32, M: u32, S: bool>(x: uN[N], y: xN[S][M][N]) -> xN[S][M];

fn or_reduce<N: u32>(x: uN[N]) -> u1;

fn priority_sel<N: u32, M: u32, S: bool>(x: uN[N], y: xN[S][M][N], z: xN[S][M]) -> xN[S][M];

fn recv_if_non_blocking<T: type>(tok: token, channel: chan<T> in, predicate: bool, value: T) -> (token, T, bool);

fn recv_if<T: type>(tok: token, channel: chan<T> in, predicate: bool, value: T) -> (token, T);

fn recv_non_blocking<T: type>(tok: token, channel: chan<T> in, value: T) -> (token, T, bool);

fn recv<T: type>(tok: token, channel: chan<T> in) -> (token, T);

fn rev<N: u32>(x: uN[N]) -> uN[N];

fn send_if<T: type>(tok: token, channel: chan<T> out, predicate: bool, value: T) -> token;

fn send<T: type>(tok: token, channel: chan<T> out, value: T) -> token;

fn signex<NS: bool, N: u32, MS: bool, M: u32>(x: xN[MS][M], y: xN[NS][N]) -> xN[NS][N];

// Note (originally in v1): the result tuple from `smulp` are two "bags of bits"
// that must be added together in order to arrive at the signed product. So we
// give them back as unsigned and users should cast the sum of these elements to
// a signed number.
fn smulp<N: u32>(x: sN[N], y: sN[N]) -> (uN[N], uN[N]);

fn token() -> token;

fn trace!<T: type>(value: T) -> T;

// `index` can be a scalar or a tuple, depending on the dimensions of `array`.
fn update<T: type, U: type, V: type, N: u32>(array: T[N], index: U, new_value: V) -> T[N];

fn umulp<N: u32>(x: uN[N], y: uN[N]) -> (uN[N], uN[N]);

fn widening_cast<DEST: type, SRC: type>(x: SRC) -> DEST;

fn xor_reduce<N: u32>(x: uN[N]) -> u1;

fn zip<LHS_TYPE: type, N: u32, RHS_TYPE: type>(lhs: LHS_TYPE[N], rhs: RHS_TYPE[N]) ->
    (LHS_TYPE, RHS_TYPE)[N];
