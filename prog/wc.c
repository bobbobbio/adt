// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <stdtyp/argparse.h>
#include <stdtyp/file.h>
#include <stdtyp/linereader.h>
#include <stdtyp/vector.h>
#include <stdtyp/string_stream.h>

#include <inttypes.h>

define_args(
    "bytes", 'c', "print the number of bytes", ARG_BOOL, ARG_OPTIONAL,
    "chars", 'm', "print the number of characters", ARG_BOOL, ARG_OPTIONAL,
    "lines", 'l', "print the number of lines", ARG_BOOL, ARG_OPTIONAL,
    "max-line-length", 'L', "print the length of the longest line", ARG_BOOL,
        ARG_OPTIONAL,
    "words", 'w', "print the number of words", ARG_BOOL, ARG_OPTIONAL,
    "version", '\0', "print version information", ARG_BOOL, ARG_OPTIONAL,
    "", '\0', "File", ARG_STRING_ARRAY, ARG_OPTIONAL
);

map_gen_podv_static(stat_map, string, uint64_t);

struct file_stats {
   struct string name;
   struct stat_map stats;
};
adt_func_static(file_stats);

static void
file_stats_init(struct file_stats *f)
{
   string_init(&f->name);
   stat_map_init(&f->stats);

   stat_map_insert(&f->stats, strw("bytes"), 0);
   stat_map_insert(&f->stats, strw("chars"), 0);
   stat_map_insert(&f->stats, strw("lines"), 0);
   stat_map_insert(&f->stats, strw("max-line-length"), 0);
   stat_map_insert(&f->stats, strw("words"), 0);
}

static void
file_stats_destroy(struct file_stats *f)
{
   string_destroy(&f->name);
   stat_map_destroy(&f->stats);
}

static void
file_stats_copy(struct file_stats *d, const struct file_stats *s)
{
   string_copy(&d->name, &s->name);
   stat_map_copy(&d->stats, &s->stats);
}

// Don't need these functions
static void
file_stats_print(const struct file_stats *f, struct string *s) {}

// Given an arguments dict, return out a string_vec with all the selected
// options
static void
get_options(struct arg_dict *dict, struct string_vec *options_out)
{
   string_vec_clear(options_out);

   // List of all the arguments, the order is the order they get printed
   create_string_vec(all_options, "lines", "words", "bytes", "chars",
      "max-line-length");

   // Go through all the arguments
   iter_value (string_vec, &all_options, option) {
      if (has_arg(dict, option))
         string_vec_append(options_out, option);
   }

   // If no options were given, use defaults
   if (string_vec_size(options_out) == 0) {
      string_vec_append(options_out, strw("lines"));
      string_vec_append(options_out, strw("words"));
      string_vec_append(options_out, strw("bytes"));
   }
}

// Print out file stats with the given options and column width
static void
file_stats_print_stats(const struct file_stats *f, int col,
   struct string_vec *options)
{
   // Print the chosen stats
   uint64_t val = *stat_map_at(&f->stats, string_vec_at(options, 0));
   aprintf("%*" PRIu64, col, val);
   for (int i = 1; i < string_vec_size(options); i++) {
      val = *stat_map_at(&f->stats, string_vec_at(options, i));
      aprintf(" %*" PRIu64, col, val);
   }

   aprintf(" %s\n", print(string, &f->name));
}

// Aggregate the data from src into dst
// XXX we special case max-line-length
static void
file_stats_append(struct file_stats *dst, const struct file_stats *src) {
   iter (stat_map, &src->stats, item) {
      if (string_equal(item.key, strw("max-line-length")))
         continue;

      *stat_map_at(&dst->stats, item.key) += *item.value;
   }

   uint64_t ml = *stat_map_at(&src->stats, strw("max-line-length"));
   if (ml > *stat_map_at(&dst->stats, strw("max-line-length")))
      *stat_map_at(&dst->stats, strw("max-line-length")) = ml;
}


static void
analyze_file(struct line_reader *lr, struct file_stats *fs_out)
{
   iter_value (line_reader, lr, line) {
      bool in_word = false;
      for (unsigned i = 0; i < string_length(line); i++) {
         if (char_is_whitespace(string_char_at_index(line, i)))
            in_word = false;
         else if (!in_word) {
            in_word = true;
            (*stat_map_at(&fs_out->stats, strw("words")))++;
         }
      }

      (*stat_map_at(&fs_out->stats, strw("lines")))++;

      uint64_t len = string_length(line);

      // This accounts for the newline character, on the last line it acconts
      // for the EOF character
      *stat_map_at(&fs_out->stats, strw("bytes")) += len + 1;
      *stat_map_at(&fs_out->stats, strw("chars")) += len + 1;

      if (len > *stat_map_at(&fs_out->stats, strw("max-line-length")))
         *stat_map_at(&fs_out->stats, strw("max-line-length")) = len;
   }
}

int
arg_main(struct arg_dict *args)
{
   // Print version and exit if --version is given
   if (has_arg(args, strw("version"))) {
      aprintf("word count v1.0\n");
      aprintf("C ADT by Remi Bernotavicius 2014");
      return EXIT_SUCCESS;
   }

   // Get the files given as args
   const struct string_vec *files = get_arg_string_array(args, strw(""));

   create(string_vec, options);
   get_options(args, &options);

   create(file_stats, total);
   string_set_cstring(&total.name, "total");

   unsigned num_stats = 0;
   static const unsigned col = 8;

   if (files == NULL) {
      // If no files are given, read stdin
      create_line_reader(lr, file_to_stream(file_stdin));

      create(file_stats, fs);
      analyze_file(&lr, &fs);

      file_stats_print_stats(&fs, col, &options);
      file_stats_append(&total, &fs);
      num_stats++;
   } else {
      // For each file given, analyze it
      iter_value (string_vec, files, file_path) {
         create(line_reader, lr);
         create(file, file);

         bool file_opened = true;
         if (string_equal(file_path, strw("-")))
            line_reader_set_stream(&lr, file_to_stream(file_stdin));
         else {
            ehandle (error, file_open(&file, file_path, O_RDONLY)) {
               afprintf(stderr, "%s: %s: %s\n", args->prog_name,
                  print(string, file_path), error_msg(error));
               file_opened = false;
            }
            if (file_opened)
               line_reader_set_stream(&lr, file_to_stream(&file));
         }

         // If we were able to open the file, analyze it
         if (file_opened) {
            create(file_stats, fs);
            analyze_file(&lr, &fs);

            string_copy(&fs.name, file_path);
            file_stats_print_stats(&fs, col, &options);
            file_stats_append(&total, &fs);
            num_stats++;
         }
      }
   }

   if (num_stats > 1)
      file_stats_print_stats(&total, col, &options);


   return EXIT_SUCCESS;
}
