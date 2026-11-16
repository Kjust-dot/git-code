#include <cstring>
#include "sha1.h"
#include "constants.h"
#include "given.h"
#include "version_control.h"
#include "structures.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

namespace {

Commit *copy_history(const Commit *source) {
  Commit *head = nullptr;
  Commit *previous = nullptr;
  Commit **next = &head;
  for (; source; source = source->next) {
    Commit *copy = new Commit(*source);
    copy->prev = previous;
    copy->next = nullptr;
    *next = copy;
    next = &copy->next;
    previous = copy;
  }
  return head;
}

void delete_history(Commit *head) {
  while (head) {
    Commit *next = head->next;
    delete head;
    head = next;
  }
}

Commit *prepend_commit(Commit *head, const User *author, const char *message,
                       time_t timestamp) {
  Commit *commit = new Commit;
  commit->author = author;
  strncpy(commit->message, message, sizeof(commit->message) - 1);
  commit->message[sizeof(commit->message) - 1] = '\0';
  commit->timestamp = timestamp;
  commit->next = head;
  commit->prev = nullptr;

  initialize(commit->hash);
  input(commit->hash, author->name, strlen(author->name));
  input(commit->hash, commit->message, strlen(commit->message));
  input(commit->hash, timestamp);
  if (head) {
    input(commit->hash, head->author->name, strlen(head->author->name));
    input(commit->hash, head->message, strlen(head->message));
    input(commit->hash, head->timestamp);
    head->prev = commit;
  }
  digest(commit->hash);
  return commit;
}

Repository *find_task_repository(const RepositoryManagement &management,
                                 const string &fqn) {
  for (int i = 0; i < management.numRepos; ++i) {
    Repository *repo = management.repos[i];
    if (fqn == string(repo->owner->name) + '/' + repo->name)
      return repo;
  }
  cout << "Repository " << fqn << " does not exist." << endl;
  return nullptr;
}

Branch *find_task_branch(Repository *repo, const string &name) {
  for (int i = 0; i < repo->numBranches; ++i) {
    if (name == repo->branches[i]->name)
      return repo->branches[i];
  }
  cout << "Branch " << repo->owner->name << '/' << repo->name << ':'
       << name << " does not exist." << endl;
  return nullptr;
}

} // namespace

/**
 * Task 1 - Register New User
 *
 * The `register_new_user` function registers a new user on the version control
 * system if no existing user has the specified username already.
 *
 * @param userManagement: reference to the `UserManagement` structure containing
 *                        the linked list of users.
 * @param username: the username of the new user to create.
 * @returns: `nullptr` if any validation failed; a pointer to the newly created
 *           user otherwise.
 */
const User *register_new_user(UserManagement &userManagement,
                              const char *username) {
  // TODO: Task 1
  if (strlen(username)>MAX_USER_NAME_LEN-1){
    cout<< "Username length exceeded "<< strlen(username)-MAX_USER_NAME_LEN+1 <<" characters."<<endl;
    return nullptr;
  }
User *cur =userManagement.head;
User *pre=nullptr;
  while(cur!=nullptr){
    if (strcmp(cur->name, username)== 0){
      cout<<"User "<<username<<" already exists."<<endl;
      return nullptr;
    }
    else if (strcmp(cur->name, username)> 0){
      break;
    }
    pre = cur;
    cur = cur->next;
  }
  
  User *newuser= new User;
  strcpy(newuser->name, username);
  newuser->numRepos=0;
  newuser->repos=nullptr;
  
  if (pre == nullptr) {
    newuser->next = userManagement.head;
    userManagement.head = newuser;
  } else {
    pre->next = newuser;
    newuser->next = cur;
  }
  return newuser;

}

/**
 * Task 2 - Create Repository
 *
 * The `create_repository` function creates a repository with the specified name
 * under the specified owner, if no existing repository under the very user has
 * the same name as specified.
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param owner: the owner of the new repository.
 * @param repoName: the name of the new repository.
 * @param creationTimestamp: the timestamp for the creation of the repository,
 *                           used for the initial commit.
 * @returns: -1 if any validation fails before repository creation; the index
 *           into the array of pointers to repositories under the owner user
 *           associated with this new repository otherwise.
 */
int create_repository(RepositoryManagement &repoManagement, User *owner,
                      const char *repoName, const time_t creationTimestamp) {
  // TODO: Task 2
  if (strlen(repoName)>=MAX_REPO_NAME_LEN){
    cout<<"Repository name length exceeded "<< strlen(repoName)-MAX_REPO_NAME_LEN+1<<" characters."<<endl;
    return -1;
  }
  for (int i = 0; i < owner->numRepos; ++i) {
        if (strcmp(owner->repos[i]->name, repoName) == 0) {
            cout << "Repository " << owner->name << "/" << repoName << " already exists." << endl;
            return -1;
        }
    }
  Repository *newrepo= new Repository;
  strcpy(newrepo->name, repoName);
  newrepo->owner=owner;
  newrepo->numBranches=0;
  newrepo->numForks=0;
  newrepo->numPrs=0;
  newrepo->prs=nullptr;
  newrepo->forks=nullptr;
  newrepo->branches=nullptr;
  newrepo->commits = prepend_commit(nullptr, owner, "Initial commit.",
                                    creationTimestamp);
  int ownerIndex = 0;
  while (ownerIndex < owner->numRepos &&
         strcmp(owner->repos[ownerIndex]->name, repoName) < 0)
    ++ownerIndex;
  Repository **newOwnerRepos = new Repository*[owner->numRepos + 1];
  for (int i = 0; i < ownerIndex; ++i)
    newOwnerRepos[i] = owner->repos[i];
  newOwnerRepos[ownerIndex] = newrepo;
  for (int i = ownerIndex; i < owner->numRepos; ++i)
    newOwnerRepos[i + 1] = owner->repos[i];
  delete[] owner->repos;
  owner->repos = newOwnerRepos;
  ++owner->numRepos;

  int count = 0;
  while (count < repoManagement.numRepos) {
    Repository *current = repoManagement.repos[count];
    int ownerOrder = strcmp(current->owner->name, owner->name);
    if (ownerOrder > 0 ||
        (ownerOrder == 0 && strcmp(current->name, repoName) > 0))
      break;
    ++count;
  }
  Repository **newallrepo=new Repository*[repoManagement.numRepos+1];
  for (int i=0; i<count;i++){
    newallrepo[i]=repoManagement.repos[i];
  }
  newallrepo[count]=newrepo;
  for (int i=count; i<repoManagement.numRepos;i++){
    newallrepo[i+1]=repoManagement.repos[i];
  }
  delete[] repoManagement.repos;
  repoManagement.repos=newallrepo;
  repoManagement.numRepos++;
  return ownerIndex;

}

/**
 * Task 3 - Create Branch
 *
 * The `create_branch` function creates a branch in the specified repository
 * with the supplied name and creator of the branch, at the specified commit,
 * if no existing branch has the same name already.
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param repoFQN: the fully-qualified name of the repository to create a branch
 *                 for.
 * @param branchName: the name of the branch to create.
 * @param creator: pointer to the creator user of the branch
 * @param commit: the commit to create the branch from.
 * @returns: false if any validation failed; true if the branch was created
 *           successfully.
 */
bool create_branch(RepositoryManagement &repoManagement, char *repoFQN,
                   const char *branchName, const User *creator,
                   const Commit *commit) {
  // TODO: Task 3
  if (strlen(branchName) >= MAX_BRANCH_NAME_LEN) {
    cout << "Branch name length exceeded "
         << strlen(branchName) - MAX_BRANCH_NAME_LEN + 1 << " characters." << endl;
    return false;
  }
  Repository *targetrepo = find_task_repository(repoManagement, repoFQN);
  if (!targetrepo)
    return false;

  bool exists = strcmp(branchName, "main") == 0;
  for (int i = 0; i < targetrepo->numBranches && !exists; ++i)
    exists = strcmp(targetrepo->branches[i]->name, branchName) == 0;
  if (exists) {
    cout << "Branch " << repoFQN << ':' << branchName << " already exists."
         << endl;
    return false;
  }
  Branch *newbranch = new Branch;
  newbranch->creator = creator;
  newbranch->repo = targetrepo;
  newbranch->head = copy_history(commit ? commit : targetrepo->commits);
  strcpy(newbranch->name, branchName);
  Branch **newbrancharray = new Branch*[targetrepo->numBranches+1];
  int insert=0;
  while (insert<targetrepo->numBranches){
    if (strcmp(targetrepo->branches[insert]->name,branchName)>0){
      break;
    }
    insert++;
  }
  for (int i=0;i<insert;i++){
    newbrancharray[i]=targetrepo->branches[i];
  }
  newbrancharray[insert]=newbranch;
  for (int i= insert; i<targetrepo->numBranches;i++){
    newbrancharray[i+1]=targetrepo->branches[i];
  }
  delete[] targetrepo->branches;
  targetrepo->branches=newbrancharray;
  targetrepo->numBranches++;
  return true;
}

/**
 * Task 4 - Add Commit
 *
 * The `add_commit` function adds a commit in the specified repository
 * with an optionally-specified branch to add the commit to. The hash
 * of the commit is computed from the author and message of the current
 * commit, as well as those of the previous commit (if any).
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param author: the author of the commit.
 * @param repoFQN: fully-qualified name of the repository to add the commit
 *                 to.
 * @param commitMessage: the message of the commit.
 * @param branch: optionally, the branch the commit is added to.
 * @param timestamp: the timestamp when the commit was created.
 */
void add_commit(RepositoryManagement &repoManagement, const User *author,
                char *repoFQN, const char *branch, const char *commitMessage,
                time_t timestamp) {
  if (timestamp == -1)
    return;
  if (strlen(commitMessage) >= MAX_COMMIT_MSG_LEN) {
    cout << "Warning: commit message is longer than " << MAX_COMMIT_MSG_LEN - 1
         << " characters and will be truncated." << endl;
  }

  Repository *repo = find_task_repository(repoManagement, repoFQN);
  if (!repo)
    return;
  const bool isMain = !branch || strcmp(branch, "main") == 0;
  Branch *target = isMain ? nullptr : find_task_branch(repo, branch);
  if (!isMain && !target)
    return;

  Commit *&head = target ? target->head : repo->commits;
  head = prepend_commit(head, author, commitMessage, timestamp);
  cout << "Pushing commit ";
  print_sha(head->hash);
  cout << " to branch " << repoFQN << ':' << (isMain ? "main" : branch)
       << '.' << endl;
}

/**
 * Task 5 - Transfer Ownership
 *
 * The `transfer_ownership` function transfers the ownership of a repository
 * from its current owner to another user. Both users have to be registered
 * users on the platform.
 *
 * @param userManagement: reference to the `UserManagement` structure containing
 *                        the linked list of users.
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param fromUsername: the name of the current owner of the specified
 *                      repsitory.
 * @param toUsername: the new owner of the specified repository.
 * @param repoName: the name of the repository.
 * @returns: true if the ownership transfer was successful; false otherwise.
 */
bool transfer_ownership(UserManagement &userManagement,
                        RepositoryManagement &repoManagement,
                        const char *fromUsername, const char *toUsername,
                        const char *repoName) {
  // TODO: Task 5
  if (strcmp(fromUsername,toUsername)==0){
    cout<<"Why are you transferring ownership to the same user?"<<endl;
    return false;
  }
  User *fromuser=nullptr;
  User *touser=nullptr;
  for (User *curuser=userManagement.head;curuser!=nullptr;curuser=curuser->next){
    if (strcmp(curuser->name, fromUsername)==0){
      fromuser=curuser;
    }
    if (strcmp(curuser->name,toUsername)==0){
      touser=curuser;
    }
  }
  if (fromuser==nullptr){
    cout<<"User "<<fromUsername<<" does not exist."<<endl;
    return false;
  }
  if (touser==nullptr){
    cout<<"User "<<toUsername<<" does not exist."<<endl;
    return false;
  }
  Repository *targetrepo =nullptr;
  int out_index=-1;
  for (int i=0;i<fromuser->numRepos;i++){
    if (strcmp(fromuser->repos[i]->name,repoName)==0){
      targetrepo=fromuser->repos[i];
      out_index=i;
      break;
    }
  }
  if (targetrepo==nullptr){
    cout<<"Repository "<<fromUsername<<'/'<<repoName<<" not found."<<endl;
    return false;
  }
  for (int i = 0; i < touser->numRepos; i++) {
    if (strcmp(touser->repos[i]->name, repoName) == 0) {
        cout << "Repository " << toUsername << "/" << repoName << " already exists." << endl;
        return false;
    }
  }
  targetrepo->owner=touser;
  Repository **newOldUserRepos = nullptr;
  if (fromuser->numRepos>1){
    newOldUserRepos=new Repository*[fromuser->numRepos-1];
    for (int i=0; i<out_index; i++){
      newOldUserRepos[i]=fromuser->repos[i];
    }
    for (int i=out_index+1; i<fromuser->numRepos;i++){
      newOldUserRepos[i-1]=fromuser->repos[i];
    }
  }
  delete[] fromuser->repos;
  fromuser->repos=newOldUserRepos;
  fromuser->numRepos--;
  Repository **newNewUserRepos =new Repository*[touser->numRepos+1];
  int insert=0;
  while (insert < touser->numRepos && strcmp(touser->repos[insert]->name, repoName) < 0) {
        insert++; 
    }
  for (int i=0;i<insert;i++){
    newNewUserRepos[i]=touser->repos[i];
  }
  newNewUserRepos[insert]=targetrepo;
  for(int i=insert; i<touser->numRepos;i++){
    newNewUserRepos[i+1]=touser->repos[i];
  }
  delete[] touser->repos;
  touser->repos=newNewUserRepos;
  touser->numRepos++;
  int globalindex=-1;
  for (int i=0;i<repoManagement.numRepos;i++){
    if (targetrepo==repoManagement.repos[i]){
        globalindex=i;
        break;
    }
  }
  for (int i=globalindex;i<repoManagement.numRepos-1;i++){
    repoManagement.repos[i]=repoManagement.repos[i+1];
    }
  int newglobalindex=0;
  while (newglobalindex<repoManagement.numRepos-1){
    Repository *curr = repoManagement.repos[newglobalindex];
      int ownerCmp = strcmp(curr->owner->name, toUsername);
      if (ownerCmp > 0) 
        break;
      if (ownerCmp == 0 && strcmp(curr->name, repoName) > 0) 
        break;
      newglobalindex++;
  }
  for (int i=repoManagement.numRepos-1;i>newglobalindex;i--){
    repoManagement.repos[i] = repoManagement.repos[i-1];
  }
  repoManagement.repos[newglobalindex]=targetrepo;
  return true;
}

/**
 * Task 6 - Create Pull Request
 *
 * The `create_pull_request` function creates a pull request from one branch of
 * a repository to another. The repositories can be different.
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param title: the creator of the pull request
 * @param author: the author of the pull request
 * @param fromBranchFQN: fully-qualified name of the branch to make a pul
 *                        request from.
 * @param toBranchFQN: fully-qualified name of the branch to merge the
 *                      suggested changes to.
 * @returns: true if the pull request was created successfully.
 */
bool create_pull_request(const RepositoryManagement &repoManagement,
                         const char *title, const User *author,
                         char *fromBranchFQN, char *toBranchFQN) {
  if (strlen(title) >= MAX_PR_TITLE_LEN) {
    cout << "Pull request title length exceeded "
         << strlen(title) - MAX_PR_TITLE_LEN + 1 << " characters." << endl;
    return false;
  }
  const string source(fromBranchFQN);
  const string target(toBranchFQN);
  const auto sourceColon = source.find(':');
  const auto targetColon = target.find(':');
  if (sourceColon == string::npos || targetColon == string::npos ||
      sourceColon == 0 || targetColon == 0 || sourceColon + 1 == source.size() ||
      targetColon + 1 == target.size())
    return false;

  const string fromRepoFQN = source.substr(0, sourceColon);
  const string toRepoFQN = target.substr(0, targetColon);
  Repository *fromRepo = find_task_repository(repoManagement, fromRepoFQN);
  if (!fromRepo)
    return false;
  Repository *toRepo = find_task_repository(repoManagement, toRepoFQN);
  if (!toRepo)
    return false;

  const string fromBranchName = source.substr(sourceColon + 1);
  const string toBranchName = target.substr(targetColon + 1);
  const bool fromMain = fromBranchName == "main";
  const bool toMain = toBranchName == "main";
  Branch *fromBranch = fromMain ? nullptr : find_task_branch(fromRepo, fromBranchName);
  if (!fromMain && !fromBranch)
    return false;
  Branch *toBranch = toMain ? nullptr : find_task_branch(toRepo, toBranchName);
  if (!toMain && !toBranch)
    return false;
  if (fromRepo == toRepo && fromBranch == toBranch) {
    cout << "Source and target branches must be different." << endl;
    return false;
  }

  PullRequest *newrequest=new PullRequest;
  newrequest->author = const_cast<User*>(author);
  newrequest->id=toRepo->numPrs+1;
  strcpy(newrequest->title,title);
  newrequest->repo=toRepo;
  newrequest->fromBranch = fromBranch;
  newrequest->toBranch = toBranch;
  newrequest->fromRepo = fromRepo;
  strcpy(newrequest->fromBranchFQN, fromBranchFQN);
  strcpy(newrequest->toBranchName, toBranchName.c_str());
  newrequest->status=OPEN;
  PullRequest **newPRarray= new PullRequest*[toRepo->numPrs+1];
  for (int i=0;i<toRepo->numPrs;i++){
    newPRarray[i]=toRepo->prs[i];
  }
  newPRarray[toRepo->numPrs]=newrequest;
  delete[] toRepo->prs;
  toRepo->prs=newPRarray;
  toRepo->numPrs++;
  cout << "Pull request #" << newrequest->id << " has been created in " << toRepoFQN << "." << endl;
  return true;
}

/** 
 * Task 7 - Fork Repository
 *
 * The `fork_repository` function allows the creation of forks of repositories.
 *
 * @param userManagement: reference to the `UserManagement` structure containing
 *                        the linked list of users.
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param owner: the owner of the repository to create the fork from.
 * @param forkedOwner: the ownr of the forked repository.
 * @param repoToFork: the name of the repository to fork.
 * @returns true if the repository was forked successfully; false otherwise.
 */
bool fork_repository(UserManagement &userManagement,
                     RepositoryManagement &repoManagement, const User *owner,
                     const char *forkedOwner, const char *repoToFork) {
  // TODO: Task 7
  Repository *baseRepo = nullptr;
  for (int i=0;i<owner->numRepos;i++){
    if (strcmp(repoToFork, owner->repos[i]->name)==0){
      baseRepo=owner->repos[i];
      break;
    }
  }
  if (baseRepo==nullptr){
    cout<<"Repository "<<owner->name<<'/'<<repoToFork<<" does not exist."<<endl;
    return false;
  }
  User *newOwner = nullptr;
  for (User *cur=userManagement.head;cur!=nullptr;cur=cur->next){
    if (strcmp(forkedOwner, cur->name)==0){
      newOwner=cur;
      break;
    }
  }
  if (newOwner==nullptr){
    cout<<"User "<<forkedOwner<<" does not exist."<<endl;
    return false;
  }
  for (int i=0;i<newOwner->numRepos;i++){
    if (strcmp(newOwner->repos[i]->name,baseRepo->name)==0){
      cout<<"Repository "<<forkedOwner<<'/'<<baseRepo->name<<" already exists."<<endl;
      return false;
    }
  }  
  Repository *newrepo=new Repository;
  strcpy(newrepo->name,repoToFork);
  newrepo->owner = newOwner;
  newrepo->numPrs=0;
  newrepo->prs=nullptr;
  newrepo->numForks=0;
  newrepo->forks=nullptr;
  newrepo->commits = copy_history(baseRepo->commits);
  newrepo->numBranches = baseRepo->numBranches;
  newrepo->branches = newrepo->numBranches ? new Branch*[newrepo->numBranches]
                                           : nullptr;
  for (int i = 0; i < newrepo->numBranches; ++i) {
    Branch *branch = new Branch(*baseRepo->branches[i]);
    branch->repo = newrepo;
    branch->head = copy_history(branch->head);
    newrepo->branches[i] = branch;
  }

  Repository **newOwnerRepos = new Repository*[newOwner->numRepos + 1];
    int pos1 = 0;
    while (pos1 < newOwner->numRepos && strcmp(newOwner->repos[pos1]->name, repoToFork) < 0) {
        pos1++;
    }
    for (int i = 0; i < pos1; i++) newOwnerRepos[i] = newOwner->repos[i];
    newOwnerRepos[pos1] = newrepo;
    for (int i = pos1; i < newOwner->numRepos; i++) newOwnerRepos[i + 1] = newOwner->repos[i];
    
    delete[] newOwner->repos;
    newOwner->repos = newOwnerRepos;
    newOwner->numRepos++;

    Repository **newForks = new Repository*[baseRepo->numForks + 1];
    int pos2 = 0;
    while (pos2 < baseRepo->numForks) {
        int cmp = strcmp(baseRepo->forks[pos2]->owner->name, forkedOwner);
        if (cmp > 0) break;
        if (cmp == 0 && strcmp(baseRepo->forks[pos2]->name, repoToFork) > 0) break;
        pos2++;
    }
    for (int i = 0; i < pos2; i++) newForks[i] = baseRepo->forks[i];
    newForks[pos2] = newrepo;
    for (int i = pos2; i < baseRepo->numForks; i++) newForks[i + 1] = baseRepo->forks[i];
    
    delete[] baseRepo->forks;
    baseRepo->forks = newForks;
    baseRepo->numForks++;

    Repository **newAllRepos = new Repository*[repoManagement.numRepos + 1];
    int pos3 = 0;
    while (pos3 < repoManagement.numRepos) {
        int cmp = strcmp(repoManagement.repos[pos3]->owner->name, forkedOwner);
        if (cmp > 0) break;
        if (cmp == 0 && strcmp(repoManagement.repos[pos3]->name, repoToFork) > 0) break;
        pos3++;
    }
    for (int i = 0; i < pos3; i++) newAllRepos[i] = repoManagement.repos[i];
    newAllRepos[pos3] = newrepo;
    for (int i = pos3; i < repoManagement.numRepos; i++) newAllRepos[i + 1] = repoManagement.repos[i];
    
    delete[] repoManagement.repos;
    repoManagement.repos = newAllRepos;
    repoManagement.numRepos++;
    cout << "Fork " << forkedOwner << "/" << repoToFork << " created successfully." << endl;
    return true;
}

namespace {

void close_if_open(PullRequest &pr) {
  if (pr.status == OPEN)
    pr.status = CLOSED;
}

void remove_source_branch(RepositoryManagement &management, Branch *branch) {
  if (!branch)
    return;

  Repository &repo = *branch->repo;
  for (int i = 0; i < management.numRepos; ++i) {
    Repository &current = *management.repos[i];
    for (int j = 0; j < current.numPrs; ++j) {
      PullRequest &pr = *current.prs[j];
      if (pr.fromBranch == branch) {
        pr.fromBranch = nullptr;
        close_if_open(pr);
      }
      if (pr.toBranch == branch) {
        pr.toBranch = nullptr;
        close_if_open(pr);
      }
    }
  }

  int index = 0;
  while (repo.branches[index] != branch)
    ++index;
  for (int i = index + 1; i < repo.numBranches; ++i)
    repo.branches[i - 1] = repo.branches[i];
  --repo.numBranches;
  if (repo.numBranches == 0) {
    delete[] repo.branches;
    repo.branches = nullptr;
  }
  delete_history(branch->head);
  delete branch;
}

vector<const Commit *> commits_to_replay(const Commit *source,
                                         const Commit *target) {
  vector<const Commit *> commits;
  for (; source; source = source->next) {
    const Commit *match = target;
    while (match && !commits_equal(*match, *source))
      match = match->next;
    if (match)
      break;
    commits.push_back(source);
  }
  return commits;
}

void merge_task_pull_request(RepositoryManagement &management, char *repoFQN,
                             int prNumber, time_t timestamp,
                             PullRequestMergeStrategy strategy) {
  if (timestamp == -1)
    return;
  Repository *repo = find_task_repository(management, repoFQN);
  if (!repo)
    return;
  if (prNumber <= 0 || prNumber > repo->numPrs) {
    cout << "Invalid pull request number for repository " << repoFQN
         << ": out of range." << endl;
    return;
  }
  PullRequest &pr = *repo->prs[prNumber - 1];
  if (pr.status != OPEN) {
    cout << "Pull request #" << prNumber << " in " << repoFQN
         << " is not open." << endl;
    return;
  }

  Commit *&target = pr.toBranch ? pr.toBranch->head : repo->commits;
  const char *method;
  if (strategy == SQUASH_AND_MERGE) {
    const string message = string(pr.title) + " (#" + to_string(prNumber) + ')';
    target = prepend_commit(target, pr.author, message.c_str(), timestamp);
    method = "squash merge";
  } else {
    const Commit *source = pr.fromBranch ? pr.fromBranch->head : pr.fromRepo->commits;
    const auto commits = commits_to_replay(source, target);
    for (auto it = commits.rbegin(); it != commits.rend(); ++it) {
      const Commit &original = **it;
      if (strategy == REBASE_MERGE) {
        target = prepend_commit(target, original.author, original.message, timestamp);
      } else {
        Commit *copy = new Commit(original);
        copy->next = target;
        copy->prev = nullptr;
        if (target)
          target->prev = copy;
        target = copy;
      }
    }

    method = "rebase merge";
    if (strategy == MERGE_COMMIT) {
      const string sourceFQN = string(pr.fromRepo->owner->name) + '/' +
                               pr.fromRepo->name + ':' +
                               (pr.fromBranch ? pr.fromBranch->name : "main");
      const string message = "Merge pull request #" + to_string(prNumber) +
                             " from branch " + sourceFQN;
      target = prepend_commit(target, pr.author, message.c_str(), timestamp);
      method = "a merge commit";
    }
  }
  pr.status = MERGED;
  remove_source_branch(management, pr.fromBranch);
  cout << "Pull request #" << prNumber << " in " << repoFQN
       << " has been merged using " << method << '.' << endl;
}

void replace_author(Commit *head, const User *user) {
  for (Commit *commit = head; commit; commit = commit->next) {
    if (commit->author == user)
      commit->author = ghost;
  }
}

void delete_repository(Repository *repo) {
  delete_history(repo->commits);
  for (int i = 0; i < repo->numBranches; ++i) {
    delete_history(repo->branches[i]->head);
    delete repo->branches[i];
  }
  for (int i = 0; i < repo->numPrs; ++i)
    delete repo->prs[i];
  delete[] repo->branches;
  delete[] repo->prs;
  delete[] repo->forks;
  delete repo;
}

} // namespace

/** Task 8.1 - Merge Pull Request (Squash Merge) */
void merge_pull_request_squashmerge(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp) {
  merge_task_pull_request(repoManagement, repoFQN, prNumber, timestamp,
                          SQUASH_AND_MERGE);
}

/** Task 8.2 - Merge Pull Request (Rebase Merge) */
void merge_pull_request_rebasemerge(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp) {
  merge_task_pull_request(repoManagement, repoFQN, prNumber, timestamp, REBASE_MERGE);
}

/** Task 8.3 - Merge Pull Request (Merge Commit) */
void merge_pull_request_mergecommit(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp) {
  merge_task_pull_request(repoManagement, repoFQN, prNumber, timestamp, MERGE_COMMIT);
}

/**
 * Task 9 - De-register User
 *
 * Reassign the user's contributions to ghost and delete owned repositories.
 */
void deregister_user(UserManagement &userManagement,
                     RepositoryManagement &repoManagement,
                     const char *username) {
  User **link = &userManagement.head;
  while (*link && strcmp((*link)->name, username) != 0)
    link = &(*link)->next;
  if (!*link) {
    cout << "User " << username << " does not exist." << endl;
    return;
  }

  User *user = *link;
  int kept = 0;
  for (int i = 0; i < repoManagement.numRepos; ++i) {
    Repository *repo = repoManagement.repos[i];
    if (repo->owner == user)
      continue;
    repoManagement.repos[kept++] = repo;

    replace_author(repo->commits, user);
    for (int j = 0; j < repo->numBranches; ++j) {
      Branch &branch = *repo->branches[j];
      if (branch.creator == user)
        branch.creator = ghost;
      replace_author(branch.head, user);
    }

    for (int j = 0; j < repo->numPrs; ++j) {
      PullRequest &pr = *repo->prs[j];
      if (pr.author == user)
        pr.author = ghost;
      if (pr.fromRepo && pr.fromRepo->owner == user) {
        pr.fromRepo = nullptr;
        pr.fromBranch = nullptr;
        close_if_open(pr);
      }
    }

    int keptForks = 0;
    for (int j = 0; j < repo->numForks; ++j) {
      if (repo->forks[j]->owner != user)
        repo->forks[keptForks++] = repo->forks[j];
    }
    repo->numForks = keptForks;
  }
  repoManagement.numRepos = kept;

  const string removedName(username);
  for (int i = 0; i < user->numRepos; ++i)
    delete_repository(user->repos[i]);
  delete[] user->repos;
  *link = user->next;
  delete user;
  cout << "User " << removedName << " has been deregistered." << endl;
}
