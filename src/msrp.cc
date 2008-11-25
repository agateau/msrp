#include <set>
#include <iostream>
#include <fstream>
#include <istream>
#include <sys/stat.h>
#include <cstdlib>
#include <string.h>

#include <cliopt.h>
#include <cliargs.h>
#include <spcre.h>

using namespace std;

static bool is_quiet;
static bool is_preserve;

string fileToString(const char *fname) {
ifstream in(fname);
return string(istreambuf_iterator<char>(in),
istreambuf_iterator<char>());
}

char * find_unused_filename(const char *startingname) {
  char *str = (char *) calloc(1, strlen(startingname) + 128);
  sprintf(str, "%s.orig", startingname);
  if (!does_file_exist_quiet(str))
    return str;
  int i;
  for (i = 2; ; i += 1) {
    sprintf(str, "%s.orig-%d", startingname, i);
    if (!is_path_file(str))
      return str;
  }
}

void process_file(SPCRE s, string repstr, string fname)
{
  string orig = fileToString(fname.c_str());
  string xformed = s.gsub(orig, repstr);
  if (orig == xformed)
    return;
  else {
    if (!is_quiet)
      cerr << "(  changed) " << fname << endl;
    string templ = fname + string("mXXXXXX");
    char *tmpfn = strdup(templ.c_str());
    ofstream outf(tmpfn);
    outf.write(xformed.c_str(), xformed.size());
    if (outf.bad()) {
      cerr << "Error, could not write file " << xformed << endl;
      cerr << "Aborting due to short write." << endl;
      exit(1);
    }
    outf.close();
    if (outf.bad()) {
      cerr << "Error, could not close file " << xformed << endl;
      cerr << "Aborting due to failed close." << endl;
      exit(1);
    }
    chmod(tmpfn, get_path_mode(fname.c_str()));
    const char *destfilename = fname.c_str();
    if (is_preserve) {
      char *origdest = find_unused_filename(destfilename);
      cout << "(preserved: " << destfilename << " => " << origdest << ")"<<endl;
      rename(destfilename, origdest);
      free(origdest);
    }
    rename(tmpfn, destfilename);
    free(tmpfn);
  }
}

void rename_file_maybe(SPCRE s, string repstr, string fname)
{
  string orig = fname;
  string xformed = s.gsub(orig, repstr);
  if (orig == xformed)
    return;
  if (!is_quiet)
    cerr << "(renamed f) " << orig << " => " << xformed << endl;
  int rc = s.options.renamer->rename(orig.c_str(), xformed.c_str(), is_preserve);
  if (rc)
    cerr << "Error renaming " << orig << " to " << xformed << endl;
}

void rename_dir_maybe(SPCRE s, string repstr, string fname)
{
  string orig = fname;
  string xformed = s.gsub(orig, repstr);
  if (orig == xformed)
    return;
  if (!is_quiet)
    cerr << "(renamed d) " << orig << " => " << xformed << endl;
  int rc = s.options.renamer->rename(orig.c_str(), xformed.c_str(), is_preserve);
  if (rc)
    cerr << "Error renaming " << orig << " to " << xformed << endl;
}

int main(int argc, const char **argv)
{
  TargetList tl;
  CliArgs ca(argv);
  deque<string>::iterator i;

  is_quiet = ca.options.is_quiet;
  is_preserve = ca.options.do_preserve;

  SPCRE s(ca.searchpat, ca.options);
  

  tl = ca.make_target_list();

  if (ca.options.do_contents) {
    for (i = tl.files.begin(); i != tl.files.end(); i++) {
      string cur = (*i);
      process_file(s,ca.repstr, cur);
    }
  }

  if (ca.options.do_filenames) {
    for (i = tl.files.begin(); i != tl.files.end(); i++) {
      string cur = (*i);
      rename_file_maybe(s, ca.repstr, cur);
    }
  }

  if (ca.options.do_directorynames) {
    for (i = tl.dirs.begin(); i != tl.dirs.end(); i++) {
      string cur = (*i);
      rename_dir_maybe(s, ca.repstr, cur);
    }
  }

  return 0;
}