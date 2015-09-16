#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - You are given the following helper functions:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the homework spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);
   
  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");
  return 0;
}



/* beargit add <filename>
 * 
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR: File <filename> already added
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR: File %s already added\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the homework 1 spec.
 *
 */

int beargit_status() {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  char strindex[FILENAME_SIZE];
  int count = 0;

  fprintf(stdout, "Tracked files:\n\n");

  while(fgets(strindex, FILENAME_SIZE, findex)){
    fprintf(stdout, "  %s", strindex);
    count++;
  }

  fclose(findex);
  fprintf(stdout, "\n%d files total\n", count);

  return 0;
}

/* beargit rm <filename>
 * 
 * See "Step 2" in the homework 1 spec.
 *
 */

int beargit_rm(const char* filename) {
  /* COMPLETE THE REST */
  FILE* findex = fopen(".beargit/.index", "r");
  FILE* fnewindex = fopen(".beargit/.newindex", "w");
  int found = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if ((line) && strcmp(line, filename) != 0) {
      fprintf(fnewindex, "%s\n", line);
    }
    else{
        found = 1;
    }
  }
  fclose(findex);
  fclose(fnewindex);

  if(!found){
    fprintf(stderr, "ERROR: File %s not tracked\n", filename);
    fs_rm(".beargit/.newindex");
    return 1;
  }

  fs_mv(".beargit/.newindex", ".beargit/.index");
  return 0;
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the homework 1 spec.
 *
 */

const char* go_bears = "GO BEARS!";

int is_commit_msg_ok(const char* msg) {
  int msg_position = 0;
  int go_bears_position = 0;
  int saving_position = 0;
  while (msg[msg_position] != '\0') {
    if (msg[msg_position] != 'G') {
      int nothing = 0;
    } else {
      saving_position = msg_position;
      go_bears_position = 0;
      while (msg[saving_position] == go_bears[go_bears_position]) {
        if (go_bears[go_bears_position + 1] == '\0') {
          return 1;
        }
        go_bears_position++;
        saving_position++;
      }      
    }
    msg_position++;
  }

  return 0;
}

char xor_char(char x, char y){
  if(x == y){
    return '1';
  }
  else{
    return '6';
  }
}

void next_commit_id_part1(char* commit_id) {
  /* COMPLETE THE REST */
  int z;
  if(commit_id[0]=='0'){
    for(z = 0; z < 30; z++){
      commit_id[z] = '6';
    }
  }
  char c1 = xor_char(commit_id[29], commit_id[27]);
  char c2 = xor_char(commit_id[26], commit_id[24]);
  char new = xor_char(c1, c2);
  
  int i;
  for(i = 29; i > 14; i--){
    commit_id[i] = commit_id[i-1];
    if(commit_id[i] == 'c'){
      commit_id[i] = '1';
    }
  }
  commit_id[14] = new;
 
  return;
}

int beargit_commit(const char* msg) {
  char check_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", check_branch, COMMIT_ID_SIZE);

  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR: Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  if (strlen(check_branch) == 0) {
    fprintf(stderr, "ERROR: Need to be on HEAD of a branch to commit\n");
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE +20];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE); //get the commit ID
  next_commit_id(commit_id); //modify to next ID

  /* COMPLETE THE REST */
  char dir_address[FILENAME_SIZE+20] = ".beargit/";
  char prev_address[FILENAME_SIZE+20] = ".beargit/.prev";
  char index_address[FILENAME_SIZE+20] = ".beargit/.index";

  char new_prev_address[FILENAME_SIZE+20]; //allocate space
  char new_index_address[FILENAME_SIZE+20];
  
  strcat(dir_address, commit_id); //generate address of new directory for the commit

  strcpy(new_prev_address, dir_address); //copy directory address to not overwrite
  strcpy(new_index_address, dir_address);
  strcat(new_prev_address, "/.prev"); //add file names onto directory address
  strcat(new_index_address, "/.index");

  fs_mkdir(dir_address); //make the directory
  strcat(dir_address, "/"); //need separator


  fs_cp(prev_address, new_prev_address); // copy .prev and .index into the new directory
  fs_cp(index_address, new_index_address);

  FILE* findex = fopen(index_address, "r"); //open index to find tracked files to copy
  char strindex[FILENAME_SIZE+20];

  while(fgets(strindex, FILENAME_SIZE, findex)){ //walk through file names in index
    char copy_address[FILENAME_SIZE+20] = "";
    strtok(strindex, "\n");
    strcat(copy_address, strindex); //add name of file
    char dest_address[FILENAME_SIZE+20];

    strcpy(dest_address, dir_address); //copy directory address to not overwrite
    strcat(dest_address, strindex); // add file name to directory address
    fs_cp(copy_address, dest_address); //copy the file
    
  }
  fclose(findex);

  strcat(dir_address, "/.msg"); //address of commit message inside new directory
  write_string_to_file(dir_address, msg); //write message to file
  write_string_to_file(prev_address, commit_id); //write .prev to .beargit/.prev

  return 0;
}

/* beargit log
 *
 * See "Step 4" in the homework 1 spec.
 * 
 */

int beargit_log(int limit) {
  /* COMPLETE THE REST */
  char prev_str[FILENAME_SIZE+20] = "/.prev";
  char msg_str[FILENAME_SIZE+20] = "/.msg";
  int count = limit;


  char prev_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", prev_id, COMMIT_ID_SIZE); //next 4 lines check if no commits
  
  if(prev_id[0] == '0'){
    fprintf(stderr, "ERROR: There are no commits!\n");
    return 1;
  }
  
  while (count > 0 && strcmp(prev_id,"0000000000000000000000000000000000000000") != 0) {
    
    fprintf(stdout, "\ncommit %s\n", prev_id);

    char prev_location[FILENAME_SIZE + 2000] = ".beargit/";
    char msg[MSG_SIZE];
    char msg_location[FILENAME_SIZE + 2000] = ".beargit/";

    strcat(prev_location, prev_id); //generates location strings
    strcat(prev_location, msg_str);
    strcat(msg_location, prev_id);
    strcat(msg_location, prev_str);

    read_string_from_file(prev_location, msg, MSG_SIZE);

    fprintf(stdout, "    %s\n", msg);

    read_string_from_file(msg_location, prev_id, COMMIT_ID_SIZE);

    // fprintf(stdout, "\ncommit %s\n    %s", prev_id, msg); //prints the current/top commit ID and message
    
    // read_string_from_file(prev_location, prev_id, COMMIT_ID_SIZE); //updates prev_id to the next one down the line

    count--;
  }
  fprintf(stdout, "\n");

  return 0;

}


const char* digits = "61c";

void next_commit_id(char* commit_id) {
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // The first COMMIT_ID_BRANCH_BYTES=10 characters of the commit ID will
  // be used to encode the current branch number. This is necessary to avoid
  // duplicate IDs in different branches, as they can have the same pre-
  // decessor (so next_commit_id has to depend on something else).
  int n = get_branch_number(current_branch);
  for (int i = 0; i < COMMIT_ID_BRANCH_BYTES; i++) {
    commit_id[i] = digits[n%3];
    n /= 3;
  }

  // Use next_commit_id to fill in the rest of the commit ID.
  next_commit_id_part1(commit_id + COMMIT_ID_BRANCH_BYTES);
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the homework 1 spec.
 *
 */

int beargit_branch() {

  char branch_gg[BRANCHNAME_SIZE];
  char line[BRANCHNAME_SIZE]; 
  char b_now[BRANCHNAME_SIZE];
  FILE *fbranches = fopen(".beargit/.branches", "r");
  read_string_from_file(".beargit/.current_branch", b_now, BRANCHNAME_SIZE);


  for (int i = 0; fgets(branch_gg, BRANCHNAME_SIZE, fbranches) != 0; i++) {
    strtok(branch_gg, "\n");
    int result = strcmp(branch_gg, b_now);
    if (result != 0) {
      printf("  %s\n", branch_gg);
    } else {
      printf("* %s\n", branch_gg);
    }
  }
  fclose(fbranches);
  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the homework 1 spec.
 *
 */



int checkout_commit(const char* commit_id) {
  FILE *file_index = fopen(".beargit/.index", "r");
  char l1[FILENAME_SIZE];
  int result = strcmp(commit_id, "0000000000000000000000000000000000000000");
  char file_name[FILENAME_SIZE];
  char new[2000] = ".beargit/";
  int flag = 1;  
  for(int i = 0; fgets(l1, sizeof(l1), file_index); i++) {
    strtok(l1, "\n");
    beargit_rm(l1);  
  }

  if(result == 0) {
    write_string_to_file(".beargit/.prev", commit_id);
    return 0;
  } else {
    FILE *file_new_index = fopen(".beargit/.index", "r");
    fs_cp(strcat(strcat(new,commit_id), "/.index"), ".beargit/.index");
    for(int j = 0; fgets(file_name, sizeof(file_name), file_new_index); j++) {
      if (flag == 1) {
        strtok(file_name, "\n");
        char copy[2000] = ".beargit/";
        fs_cp(strcat(strcat(strcat(copy,commit_id), "/"), file_name), file_name);        
      } else {
        int nothing = 0;
        // do nothing
      }

    }
    fclose(file_new_index); 
  }

  fclose(file_index);
  write_string_to_file(".beargit/.prev", commit_id);
  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  if (commit_id[COMMIT_ID_BYTES] == '\0') {
    for (int i = 0; i < COMMIT_ID_BYTES; i++) {
      char cur_str = commit_id[i];
      if (cur_str == 'c' || cur_str == '1' || cur_str == '6') {
        continue;
      } else {
        return 0;
      }
    }
    return 1;
  } else {
    return 0;
  }
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);

  // If not detached, update the current branch by storing the current HEAD into that branch's file...
  // Even if we cancel later, this is still ok.
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

  // Check whether the argument is a commit ID. If yes, we just stay in detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    if (!fs_check_dir_exists(commit_dir)) {
      fprintf(stderr, "ERROR: Commit %s does not exist\n", arg);
      return 1;
    }

    // Set the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(branch_name) >= 0);

  // Check for errors.
  if (!(!branch_exists || !new_branch)) {
    fprintf(stderr, "ERROR: A branch named %s already exists\n", branch_name);
    return 1;
  } else if (!branch_exists && !new_branch) {
    fprintf(stderr, "ERROR: No branch %s exists\n", branch_name);
    return 1;
  }

  // File for the branch we are changing into.
  char branch_file[FILENAME_SIZE] =  ".beargit/.branch_"; 
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file); 
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}
