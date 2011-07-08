/* libhyphenate: A TeX-like hyphenation algorithm.
 * Copyright (C) 2007 Steve Wolter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * If you have any questions, feel free to contact me:
 * http://swolter.sdf1.org
 **/

/* ------------- Implementation for HyphenationTree.h ---------------- */

#include "HyphenationTree.h"
#include <glib.h>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace Hyphenate;

/* The HyphenationNode is a tree node for the hyphenation search tree. It
* represents the matching state after a single character; if there is a
* pattern that ends with that particular character, the hyphenation_pattern
* is set to non-NULL. The jump_table links to the children of that node,
* indexed by letters. */
class Hyphenate::HyphenationNode {
   public:
      typedef std::map<char, HyphenationNode*> JumpTable;
      /* Table of children */
      JumpTable jump_table;
      /* Hyphenation pattern associated with the full path to this node. */
      std::auto_ptr<HyphenationRule> hyphenation_pattern;

      HyphenationNode() {}
      ~HyphenationNode() {
         /* The destructor has to destroy all childrens. */
         for (JumpTable::iterator i = jump_table.begin();
               i != jump_table.end(); i++)
            delete i->second;
      }

      /** Find a particular jump table entry, or NULL if there is 
         * none for that letter. */
      inline const HyphenationNode *find(char arg) const {
         JumpTable::const_iterator i = jump_table.find(arg);
         if (i != jump_table.end()) return i->second; else return NULL;
      }
      /** Find a particular jump table entry, or NULL if there is none 
         * for that letter. */
      inline HyphenationNode *find(char arg) {
         JumpTable::iterator i = jump_table.find(arg);
         if (i != jump_table.end()) return i->second; else return NULL;
      }

      /** Insert a particular hyphenation pattern into this 
         *  hyphenation subtree.
      * \param pattern The character pattern to match in the input word.
      * \param hp The digit-pattern for the hyphenation algorithm.
      */
      void insert (const char *id, 
         std::auto_ptr<HyphenationRule> pattern);

      /** Apply all patterns for that subtree. */
      void apply_patterns(
         char *priority_buffer, 
         const HyphenationRule ** rule_buffer, 
         const char *to_match) const;
};

Hyphenate::HyphenationTree::HyphenationTree() : 
   root(new HyphenationNode()), start_safe(1), end_safe(1) {}

Hyphenate::HyphenationTree::~HyphenationTree() {
   delete root;
}

void Hyphenate::HyphenationTree::insert(auto_ptr<HyphenationRule> pattern) {
   /* Convert our key to lower case to ease matching. */
   const char *upperCaseKey = pattern->getKey().c_str();
   gunichar *gucs = g_utf8_to_ucs4_fast(upperCaseKey, -1, NULL);
   for (int i = 0; gucs[i] != 0; i++)
      gucs[i] = g_unichar_tolower(gucs[i]);
   gchar *gs = g_ucs4_to_utf8(gucs, -1, NULL, NULL, NULL);
   g_free(gucs);

   root->insert(gs, pattern);
   g_free(gs);
}

void HyphenationNode::insert (const char* key_string, 
                              auto_ptr<HyphenationRule> pattern) 
{
   /* Is this the terminal node for that pattern? */
   if (key_string[0] == 0) {
      /* If we descended the tree all the way to the last letter, we can now
       * write the pattern into this node. */

      hyphenation_pattern.reset(pattern.release());
   } else  {
      /* If not, however, we make sure that the branch for our letter exists
       * and descend. */
      char key = key_string[0];
      /* Ensure presence of a branch for that letter. */
      HyphenationNode *p = find(key);
      if (!p) {
	 p = new HyphenationNode();
	 jump_table.insert(pair<char, HyphenationNode*>(key, p));
      }
      /* Go to the next letter and descend. */
      p->insert(key_string+1, pattern);
   }
}

void Hyphenate::HyphenationNode::apply_patterns(
   char *priority_buffer, 
   const HyphenationRule ** rule_buffer, 
   const char *to_match) const
{
   /* First of all, if we can descend further into the tree (that is,
    * there is an input char left and there is a branch in the tree),
    * do so. */
   char key = to_match[0];

   if (key != 0) {
      const HyphenationNode *next = find(key);
      if ( next != NULL )
         next->apply_patterns(priority_buffer, rule_buffer, to_match+1);
   }

   /* Now, if we have a pattern at this point in the tree, it must be a good
    * match. Apply the pattern. */
   const HyphenationRule* hyp_pat = hyphenation_pattern.get();
   if (hyp_pat != NULL)
      for (int i = 0; hyp_pat->hasPriority(i); i++)
	 if (priority_buffer[i] < hyp_pat->priority(i)) {
            rule_buffer[i] = (hyp_pat->priority(i) % 2 == 1) ? hyp_pat : NULL;
            priority_buffer[i] = hyp_pat->priority(i);
         }
}

auto_ptr<vector<const HyphenationRule*> > HyphenationTree::applyPatterns
   (const string &word) const
{
   return applyPatterns(word, string::npos);
}

auto_ptr<vector<const HyphenationRule*> > HyphenationTree::applyPatterns
   (const string &word, size_t stop_at) const
{
   /* Prepend and append a . to the string (word start and end), and convert
    * all characters to lower case to ease matching. */
   std::string w = ".";
   {
      gunichar *gs = g_utf8_to_ucs4_fast(word.c_str(), -1, NULL);
      for (int i = 0; gs[i] != 0; i++)
         gs[i] = g_unichar_tolower(gs[i]);
      gchar *lowerChars = g_ucs4_to_utf8(gs, -1, NULL, NULL, NULL);
      g_free(gs);
      w += lowerChars;
      g_free(lowerChars);
   }
   w += ".";

   /* Vectors for priorities and rules. */
   vector<char> pri(w.size()+2,0);
   vector<const HyphenationRule*> rules(w.size()+1, NULL);

   /* For each suffix of the expanded word, search all matching prefixes.
    * That way, each possible match is found. Note the pointer arithmetics
    * in the first and second argument. */
   for (int i = 0; i < w.size()-1 && i <= stop_at; i++)
      root->apply_patterns((&pri[i]), (&rules[i]), w.c_str() + i);

   /* Copy the results to a shorter vector. */
   auto_ptr<vector<const HyphenationRule*> > output_rules(
      new vector<const HyphenationRule*>(word.size(), NULL));
   
   /* We honor the safe areas at the start and end of each word here. */
   /* Please note that the incongruence between start and end is due
    * to the fact that hyphenation happens _before_ each character. */
   int ind_start = 1, ind_end = w.size()-1;
   for (int skip = 0; skip < start_safe && ind_start < w.size(); ind_start++)
      if ((w[ind_start] & 0xC0) != 0x80)
         skip++;
   for (int skip = 0; skip < end_safe && ind_end > 0; ind_end--)
      if ((w[ind_end] & 0xC0) != 0x80)
         skip++;

   for (int i = ind_start; i <= ind_end; i++)
      (*output_rules)[i-1] = rules[i];
   return output_rules;
}

void HyphenationTree::loadPatterns(istream &i) {
   string pattern;
   /* The input is a file with whitespace-separated words.
    * The first numerical-only word we encountered denotes the safe start,
    * the second the safe end area. */

   char ch;
   bool numeric = true;
   int num_field = 0;
   while ( i.get(ch) ) {
      if (ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
	 /* The output operation. */
         if (pattern.size() && numeric && num_field <= 1) {
            ((num_field == 0) ? start_safe : end_safe) = atoi(pattern.c_str());
            num_field++;
	 } else if (pattern.size()) {
            if ( ! g_utf8_validate(pattern.c_str(), -1, NULL) )
               throw std::domain_error(
                  "Hyphenation pattern files must be UTF-8 encoded. " +
                  ("The pattern " + pattern) + " is not.");

            insert(
               auto_ptr<HyphenationRule>(new HyphenationRule(pattern)));
         }

	 /* Reinitialize state. */
	 pattern.clear();
         numeric = true;
      } else {
	 /* This rule catches all other (mostly alpha, but probably UTF-8)
	  * characters. It normalizes the previous letter and then appends
          * it to the pattern. */
         pattern += ch;
         if (ch < '0' || ch > '9') numeric = false;
      }
   }

   if (pattern.size()) 
      insert(auto_ptr<HyphenationRule>(new HyphenationRule(pattern)));
}

