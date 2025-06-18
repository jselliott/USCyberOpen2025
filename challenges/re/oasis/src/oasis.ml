external mprotect : nativeint -> int -> int -> int = "caml_mprotect"

let page_size = 4096
let prot_read = 0x1
let prot_write = 0x2
let prot_exec = 0x4

let rec lcg_stream seed a c m len =
  let rec aux x acc n =
    if n = 0 then List.rev acc
    else
      let x' = (a * x + c) mod m in
      aux x' ((x' land 0xFF) :: acc) (n - 1)
  in
  aux seed [] len

let decrypt_lcg ~seed ~a ~c ~m data =
  let ks = lcg_stream seed a c m (Bytes.length data) in
  List.iteri (fun i k ->
    Bytes.set data i (Char.chr (Char.code (Bytes.get data i) lxor k))
  ) ks;
  data

open Bigarray

let () =
  let ch = open_in_bin "payload.enc" in
  let len = in_channel_length ch in
  let enc = Bytes.create len in
  really_input ch enc 0 len;
  close_in ch;

  let decrypted = decrypt_lcg ~seed:0x1337 ~a:1103515245 ~c:12345 ~m:(1 lsl 31) enc in

  let buf = Array1.create char c_layout len in
  for i = 0 to len - 1 do
    buf.{i} <- Bytes.get decrypted i
  done;

  let addr = Bigarray.Array1.start buf |> Nativeint.of_int in
  ignore (mprotect addr len (prot_read lor prot_exec));

  let func : (string -> int) = Obj.magic (Bigarray.Array1.start buf) in
  print_string "Enter flag: ";
  let input = read_line () in
  let result = func input in
  if result = 1 then print_endline "Correct!" else print_endline "Wrong!"
