#include "spcre.h"

#include <iostream>
#include <string>

using namespace std;

string SPCREResult::backsub(string origstr)
{
  int curoffset = 0;
  int len = origstr.size();
  string res;
  const char *cs = origstr.c_str();
  while (curoffset < len) {
    int slashind = origstr.find('\\', curoffset);
    if (slashind != int(string::npos)) {
      res.append(cs + curoffset, slashind - curoffset);
      if (slashind + 1 < len) {
        char c = cs[slashind+1];
        if (c >= '0' && c <= '9') {
          int whichnum = c - '0';
          if (whichnum <= int(cap.size()))
            res.append(cap[whichnum]);
        }
      }
      curoffset = slashind + 2;
    }
    else {
      res.append(cs + curoffset, len - curoffset);
      break;
    }
  }
  return res;
}

string SPCRE::gsub(string origstr, string repstr)
{
  const bool first_match_only = options.first_match_only;
  int curoffset = 0;
  int len = origstr.size();
  string res;
  const char *cs = origstr.c_str();
  while (curoffset < len) {
    SPCREResult r = match(origstr, curoffset);
    if (r.didmatch) {
      res.append(cs + curoffset, r.start_offset - curoffset);
      res.append(r.backsub(repstr));
      curoffset = r.pastend_offset;
      if (first_match_only) {
        res.append(cs+curoffset, len - curoffset); 
        break; 
      }
    }
    else {
      res.append(cs+curoffset, len - curoffset);
      break;
    }
  }
  return res;
}

SPCREResult SPCRE::match(string inpstr, int offset)
{
  int rc;
  SPCREResult res;
  res.didmatch = false;
  res.didfinish = false;
  const char *cs = inpstr.c_str();
  pcre2_match_data *mdata = pcre2_match_data_create_from_pattern(prex, NULL);
  rc = pcre2_match(prex, (PCRE2_SPTR)cs, inpstr.size(), offset, 0, mdata, NULL);
  if (rc > 0) {
    res.didmatch = true;
  } else {
    res.didfinish = true;
  }
  if (res.didmatch) {
    int i;
    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(mdata);
    res.start_offset = ovector[0];
    res.pastend_offset = ovector[1];
    if (res.pastend_offset >= int(inpstr.size()))
      res.didfinish = true;
    for (i = 0; i < rc; i += 1) {
      PCRE2_SIZE ostart = ovector[2*i], opastend = ovector[2*i+1];
      if (ostart == PCRE2_UNSET || opastend == PCRE2_UNSET) {
        /* Group did not participate in the match */
        res.cap.push_back(string());
        continue;
      }
      if (opastend < ostart) {
        cerr << "Error on offset " << i << " : " << ostart << "," << opastend << endl;
        exit(1);
      }
      res.cap.push_back(string(cs + ostart, opastend - ostart));
    }
  }
  pcre2_match_data_free(mdata);
  return res;
}

SPCRE::SPCRE(string pattern, CliOpt co)
{
  options = co;
  pat = pattern;
  prex = get_pcre();
}

pcre2_code *SPCRE::get_pcre(void)
{
  pcre2_code *result;
  uint32_t optval = PCRE2_MULTILINE;
  int errcode;
  PCRE2_SIZE erroffset;
  string curpat = pat;
  if (options.word_boundary)
    curpat = string("\\b") + curpat + string("\\b");
  if (options.case_insensitive)
    optval |= PCRE2_CASELESS;
  result = pcre2_compile((PCRE2_SPTR)curpat.c_str(), PCRE2_ZERO_TERMINATED,
                         optval, &errcode, &erroffset, NULL);
  if (result == 0) {
    PCRE2_SIZE i;
    PCRE2_UCHAR errbuf[256];
    pcre2_get_error_message(errcode, errbuf, sizeof(errbuf));
    cerr << "Error in pattern, offset " << erroffset << ": " << errbuf << endl;
    cerr << curpat << endl;
    for (i = 0; i < erroffset; i += 1)
      cerr << ' ';
    cerr << "^" << endl;
    exit(1);
  }
  return result;
}
