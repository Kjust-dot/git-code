#include <cstring>
#include "sha1.h"
#include "constants.h"
#include "given.h"
#include "pa3_task.h"
#include "structures.h"
#include <iostream>
#include <cstdio>
using namespace std;

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
    cout<< "Username length exceeded "<< strlen(username)-MAX_USER_NAME_LEN+2 <<" characters."<<endl;
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
    cout<<"Repository name length exceeded "<< strlen(repoName)-MAX_REPO_NAME_LEN+5<<" characters."<<endl;
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
  Commit *Initial_commit=new Commit;
  Initial_commit->author=owner;
  strcpy(Initial_commit->message, "Initial commit.");
  Initial_commit->timestamp=creationTimestamp;
  initialize(Initial_commit->hash);
  input(Initial_commit->hash, owner->name,strlen(owner->name));
  input(Initial_commit->hash,Initial_commit->message,strlen(Initial_commit->message));
  input(Initial_commit->hash,Initial_commit->timestamp);
  digest(Initial_commit->hash);
  Initial_commit->next=nullptr;
  Initial_commit->prev=nullptr;
  newrepo->commits=Initial_commit;
  Repository **newownrepo = new Repository*[owner->numRepos+1];
  for (int i=0;i< owner->numRepos;i++){
    newownrepo[i]=owner->repos[i];
  }
  newownrepo[owner->numRepos]=newrepo;
  delete[] owner->repos;
  owner->repos=newownrepo;
  owner->numRepos++;
  Repository **cur=repoManagement.repos;
  Repository **end=repoManagement.repos+repoManagement.numRepos;
  int count=0;
  while(cur<end){
    Repository *now=*cur; 
    if (strcmp(now->owner->name,newrepo->owner->name)>0){
      break;
      }
      else if (strcmp(now->owner->name,newrepo->owner->name)==0){
        if (strcmp(now->name,newrepo->name)>0){
          break;
        }
      }
    cur++;
    count++;
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
  return count;
  /*return owner->numRepos-1*/ 

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
  Repository *targetrepo=nullptr;
  if (strlen(branchName)>MAX_BRANCH_NAME_LEN){
    cout<<"Branch name length exceeded "<<strlen(branchName)-MAX_BRANCH_NAME_LEN+1<<" characters."<<endl;
    return false;
  }
  for (int i=0; i<repoManagement.numRepos;i++){
    Repository *currepo=repoManagement.repos[i];
    char tempFQN[MAX_USER_NAME_LEN + MAX_REPO_NAME_LEN + 2];
    strcpy(tempFQN, currepo->owner->name);
    strcat(tempFQN, "/");
    strcat(tempFQN, currepo->name);
    if (strcmp(tempFQN,repoFQN)==0){
      targetrepo=currepo;
      break;
    }
  }
  if (targetrepo==nullptr){
    cout<<"Repository "<<repoFQN<<" does not exist."<<endl;
    return false;
  }
  for (int i=0;i<targetrepo->numBranches;i++){
    if (strcmp(targetrepo->branches[i]->name,branchName)==0){
      cout<<"Branch "<<repoFQN<<':'<<branchName<<" already exists."<<endl;
      return false;
    }
  }
  const Commit *sourcecommit=commit;
  if (sourcecommit==nullptr){
    sourcecommit=targetrepo->commits;
  }
  Commit *newcommit=new Commit;
  *newcommit=*sourcecommit;
  Branch *newbranch=new Branch;
  newbranch->creator=const_cast<User*>(creator);
  newbranch->repo=targetrepo;
  newbranch->head=newcommit;
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
  // TODO: Task 4
  char finalcommit[MAX_COMMIT_MSG_LEN];
  if (strlen(commitMessage)>MAX_COMMIT_MSG_LEN-1  ){
    cout<<"Warning: commit message is longer than "<<MAX_COMMIT_MSG_LEN - 1<<" characters and will be truncated."<<endl;
    for(int i=0; i<MAX_COMMIT_MSG_LEN-1;i++){
      finalcommit[i]=commitMessage[i];
    }
    finalcommit[MAX_COMMIT_MSG_LEN-1]='\0';  
  }
  else {
    strcpy(finalcommit,commitMessage);
  }
  Repository *targetrepo = nullptr;
  for (int i=0; i<repoManagement.numRepos;i++){
    Repository *currepo=repoManagement.repos[i];
    char tempFQN[MAX_USER_NAME_LEN + MAX_REPO_NAME_LEN + 2];
    strcpy(tempFQN, currepo->owner->name);
    strcat(tempFQN, "/");
    strcat(tempFQN, currepo->name);
    if (strcmp(tempFQN,repoFQN)==0){
      targetrepo=currepo;
      break;
    }
  }
  if (targetrepo==nullptr){
    cout<<"Repository "<<repoFQN<<" does not exist."<<endl;
    return;
  }
  Branch *targetbranch=nullptr;
  bool found=false;
  if (branch != nullptr){
    for (int i=0; i<targetrepo->numBranches;i++){
      if (strcmp(branch,targetrepo->branches[i]->name)==0){
        targetbranch=targetrepo->branches[i];
        found=true;
        break;
      }
    }
  }
  if (branch != nullptr && found == false){
    cout<<"Branch "<<repoFQN<<':'<<branch<<" does not exist."<<endl;
    return;
  }
  Commit *precommit= nullptr;
  if (branch==nullptr){
    precommit=targetrepo->commits;
  }
  else{
    precommit=targetbranch->head;
  }
  Commit *newcommit=new Commit;
  newcommit->author = const_cast<User*>(author);
  strcpy(newcommit->message, finalcommit);
  newcommit->timestamp=timestamp;
  initialize(newcommit->hash);
  input(newcommit->hash,newcommit->author->name,strlen(newcommit->author->name));
  input(newcommit->hash,newcommit->message,strlen(newcommit->message));
  input(newcommit->hash,newcommit->timestamp);
  if (precommit!=nullptr){
  input(newcommit->hash,precommit->author->name,strlen(precommit->author->name));
  input(newcommit->hash,precommit->message,strlen(precommit->message));
  input(newcommit->hash,precommit->timestamp);
  }
  digest(newcommit->hash);
  if (precommit != nullptr) {
     precommit->prev = newcommit;
    }
  newcommit->prev=nullptr;
  newcommit->next=precommit;
  if (branch!=nullptr){
    targetbranch->head=newcommit;
  }
  else{
    targetrepo->commits=newcommit;
  }
  cout<<"Pushing commit ";
  print_sha(newcommit->hash);
  const char *branchOutputName = (branch != nullptr) ? branch : "main";
  cout<<" to branch "<<repoFQN<<':'<<branchOutputName<<'.'<<endl;

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
  // TODO: Task 6
  if (strlen(title)>MAX_PR_TITLE_LEN-1){
    cout<<"Pull request title length exceeded "<<strlen(title)-MAX_PR_TITLE_LEN+28<<" characters."<<endl;
    return false;
  }
  char fromRepoFQN[256];
  char fromBranchName[256];
  int colonpos1=-1;
  for (int i=0;i<strlen(fromBranchFQN)-1;i++){
    if (fromBranchFQN[i] == ':') {
        colonpos1 = i;
        break;
    }
  }
  if (colonpos1==-1){
    return false;
  }
  for (int i=0;i<colonpos1;i++){
    fromRepoFQN[i]=fromBranchFQN[i];
  }
  fromRepoFQN[colonpos1] = '\0';
  int j=0;
  for (int i=colonpos1+1 ;i<strlen(fromBranchFQN);i++){
    fromBranchName[j]=fromBranchFQN[i];
    j++;
  }
  fromBranchName[j] = '\0';
  char toRepoFQN[256];
  char toBranchName[256];
  int colonpos2=-1;
  for (int i=0;i<strlen(toBranchFQN)-1;i++){
    if (toBranchFQN[i] == ':') {
        colonpos2 = i;
        break;
    }
  }
  if (colonpos2==-1){
    return false;
  }
  for (int i=0;i<colonpos2;i++){
    toRepoFQN[i]=toBranchFQN[i];
  }
  toRepoFQN[colonpos2] = '\0';
  int m=0;
  for (int i=colonpos2+1;i<strlen(toBranchFQN);i++){
    toBranchName[m]=toBranchFQN[i];
    m++;
  }
  toBranchName[m] = '\0';
  Repository *fromRepo = nullptr;
  Repository *toRepo = nullptr;


  for (int i=0;i<repoManagement.numRepos;i++){
    Repository *cur=repoManagement.repos[i];
    char tempfqn[256];
    strcpy(tempfqn, cur->owner->name);
    strcat(tempfqn, "/");
    strcat(tempfqn, cur->name);
    if (strcmp(tempfqn,fromRepoFQN)==0){
      fromRepo=cur;
    }
    if (strcmp(tempfqn,toRepoFQN)==0){
      toRepo=cur;
    }
  }
  if (fromRepo==nullptr){
    cout<<"Repository "<<fromRepoFQN<<" does not exist."<<endl;
    return false;
  }
  if (toRepo==nullptr){
    cout<<"Repository "<<toRepoFQN<<" does not exist."<<endl;
    return false;
  }
  Branch *fromBranchprt=nullptr;
  Branch *toBranchprt=nullptr;
  if (strcmp(fromBranchName, "main") != 0){
    for (int i=0;i<fromRepo->numBranches;i++){
      if (strcmp(fromRepo->branches[i]->name,fromBranchName)==0){
        fromBranchprt=fromRepo->branches[i];
        break;
      }
    }
    if (fromBranchprt==nullptr){
      cout<<"Branch "<<fromBranchFQN<<" does not exist."<<endl;
      return false;
    }
  }
  if (strcmp(toBranchName, "main") != 0){
    for (int i=0;i<toRepo->numBranches;i++){
    if (strcmp(toRepo->branches[i]->name,toBranchName)==0){
        toBranchprt=toRepo->branches[i];
        break;
      }
    }
    if (toBranchprt==nullptr){
      cout<<"Branch "<<toBranchFQN<<" does not exist."<<endl;
      return false;
    }
  }
  PullRequest *newrequest=new PullRequest;
  newrequest->author = const_cast<User*>(author);
  newrequest->id=toRepo->numPrs+1;
  strcpy(newrequest->title,title);
  newrequest->repo=toRepo;
  newrequest->fromBranch=fromBranchprt;
  newrequest->toBranch=toBranchprt;
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
  Commit *oldcur=baseRepo->commits;
  Commit *newhead=nullptr;
  Commit *newtail=nullptr;
  while (oldcur!=nullptr){
    Commit *newCommit = new Commit;
    *newCommit=*oldcur;
    newCommit->next=nullptr;
    newCommit->prev=newtail;
    if (newtail==nullptr){
      newhead=newCommit;
    }
    else{
      newtail->next=newCommit;
    }
    newtail=newCommit;
    oldcur=oldcur->next;
  }
  newrepo->commits = newhead;
  newrepo->numBranches = baseRepo->numBranches;
  if (newrepo->numBranches>0){
    newrepo->branches = new Branch*[newrepo->numBranches];
    for (int i=0; i<newrepo->numBranches;i++){
      Branch *oldBranch = baseRepo->branches[i];
      Branch *newBranch = new Branch;
      strcpy(newBranch->name,oldBranch->name);
      newBranch->creator=oldBranch->creator;
      newBranch->repo=newrepo;
      Commit *oldBranchCurr = oldBranch->head;
      Commit *newBranchHead = nullptr;
      Commit *newBranchTail = nullptr;

      while (oldBranchCurr != nullptr) {
        Commit *searchNew = newrepo->commits;
        bool foundInMain = false;
          
        while (searchNew != nullptr) {
          if (searchNew->timestamp == oldBranchCurr->timestamp &&
            strcmp(searchNew->message, oldBranchCurr->message) == 0) {
            foundInMain = true;
            break;
          }
            searchNew = searchNew->next;
        }
        if (foundInMain) {
          if (newBranchTail != nullptr) {
            newBranchTail->next = searchNew;
          } else {
            newBranchHead = searchNew; 
            }
            break; 
        } else {
          Commit *newCommit = new Commit;
          *newCommit = *oldBranchCurr;
          newCommit->next = nullptr;
          newCommit->prev = newBranchTail;
          if (newBranchTail != nullptr) {
            newBranchTail->next = newCommit;
          } else {
            newBranchHead = newCommit;
            }
          newBranchTail = newCommit;
          oldBranchCurr = oldBranchCurr->next;
        }
      }
      newBranch->head = newBranchHead;
      newrepo->branches[i]=newBranch;
    }
  }
  else{
    newrepo->branches=nullptr;
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

/**
 * Task 8.1 - Merge Pull Request (Squash Merge)
 *
 * The `merge_pull_request_squashmerge` function merges the specified
 * pull request in a repsitory using the squash merge strategy
 * (combines all commits in the pull request into one and add it to the target
 * branch).
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param repoFQN: the name of the repository to merge a pull request for.
 * @param prNumber: the number of the pull request to merge.
 * @param timestamp: the timestamp when this pull request was merged.
 */
void merge_pull_request_squashmerge(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp) {
  // TODO: Task 8.1
  Repository *targetrepo = nullptr;
  for (int i=0; i<repoManagement.numRepos;i++){
    Repository *currepo=repoManagement.repos[i];
    char tempFQN[MAX_USER_NAME_LEN + MAX_REPO_NAME_LEN + 2];
    strcpy(tempFQN, currepo->owner->name);
    strcat(tempFQN, "/");
    strcat(tempFQN, currepo->name);
    if (strcmp(tempFQN,repoFQN)==0){
      targetrepo=currepo;
      break;
    }
  }
  if (targetrepo==nullptr){
    cout<<"Repository "<<repoFQN<<" does not exist."<<endl;
    return;
  }
  if (prNumber>targetrepo->numPrs){
    cout << "Invalid pull request number for repository " << repoFQN << ": out of range." << endl;
    return;
  }
  PullRequest *pr = targetrepo->prs[prNumber - 1];
  char commitMsg[MAX_COMMIT_MSG_LEN];
  sprintf(commitMsg,"%s (#%d)",pr->title,prNumber);
  Commit *newcommit=new Commit;
  newcommit->author=pr->author;
  newcommit->timestamp=timestamp;
  strcpy(newcommit->message,commitMsg);
  Commit *precommit= nullptr;
  if (pr->toBranch==nullptr){
    precommit=targetrepo->commits;
  }
  else{
    precommit=pr->toBranch->head;
  }
  initialize(newcommit->hash);
  input(newcommit->hash,newcommit->author->name,strlen(newcommit->author->name));
  input(newcommit->hash,newcommit->message,strlen(newcommit->message));
  input(newcommit->hash,newcommit->timestamp);
  if (precommit!=nullptr){
  input(newcommit->hash,precommit->author->name,strlen(precommit->author->name));
  input(newcommit->hash,precommit->message,strlen(precommit->message));
  input(newcommit->hash,precommit->timestamp);
  }
  digest(newcommit->hash);
  if (precommit != nullptr) {
     precommit->prev = newcommit;
    }
  newcommit->prev=nullptr;
  newcommit->next=precommit;
  if (pr->toBranch==nullptr){
    targetrepo->commits=newcommit;
  }
  else{
    pr->toBranch->head=newcommit;
  }
  if (pr->fromBranch!=nullptr){
    Repository *fromRepo = pr->fromBranch->repo;
    Commit *cur = pr->fromBranch->head;
    while (cur!=nullptr){
      bool isshared=false;
      Commit *mc = fromRepo->commits; 
      while(mc != nullptr){
        if (cur == mc){
          isshared = true;
          break;
        }
        mc = mc->next; 
      }
      if (isshared==false){
        for (int i=0; i<fromRepo->numBranches;i++){
          Branch *b= fromRepo->branches[i];
          if (pr->fromBranch==b){
            continue;
          }
          Commit *bc= b->head;
          while (bc!=nullptr){
            if (bc==cur){
              isshared=true;
              break;
            }
            bc=bc->next;
          }
        }
        if (isshared){
          break;
        }
      }  
      if (isshared){
        break;
      }
      Commit *todelete=cur;
      cur=cur->next;
      if (cur!=nullptr){
        cur->prev=nullptr;
      }
      delete todelete;
    }
    int bIdx = -1;
    for (int i = 0; i < fromRepo->numBranches; i++) {
      if (fromRepo->branches[i] == pr->fromBranch) {
        bIdx = i;
        break;
      }
    }
    if (bIdx != -1) {
      delete fromRepo->branches[bIdx];
      for (int i = bIdx; i < fromRepo->numBranches - 1; i++) {
        fromRepo->branches[i] = fromRepo->branches[i + 1];
      }
      fromRepo->numBranches--;
      if (fromRepo->numBranches == 0) {
        delete[] fromRepo->branches;
        fromRepo->branches = nullptr;
      } 
      else {
        Branch **newBranches = new Branch*[fromRepo->numBranches];
          for (int i = 0; i < fromRepo->numBranches; i++) {
            newBranches[i] = fromRepo->branches[i];
          }
          delete[] fromRepo->branches;
          fromRepo->branches = newBranches;
      }
    }
    pr->fromBranch = nullptr; 
  }
  pr->status = MERGED; 
  cout << "Pull request #" << prNumber << " in " << repoFQN << " has been merged using squash merge." << endl;
}

/**
 * Task 8.2 - Merge Pull Request (Rebase Merge)
 *
 * The `merge_pull_request_rebasemerge` function merges the specified
 * pull request in a repsitory using the rebase merge strategy
 * (rebases all commits in the pull request to the target branch).
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param repoFQN: the name of the repository to merge a pull request for.
 * @param prNumber: the number of the pull request to merge.
 * @param timestamp: the timestamp when this pull request was merged.
 */
void merge_pull_request_rebasemerge(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp) {
  // TODO: Task 8.2
  Repository *targetrepo = nullptr;
  for (int i=0; i<repoManagement.numRepos;i++){
    Repository *currepo=repoManagement.repos[i];
    char tempFQN[MAX_USER_NAME_LEN + MAX_REPO_NAME_LEN + 2];
    strcpy(tempFQN, currepo->owner->name);
    strcat(tempFQN, "/");
    strcat(tempFQN, currepo->name);
    if (strcmp(tempFQN,repoFQN)==0){
      targetrepo=currepo;
      break;
    }
  }
  if (targetrepo==nullptr){
    cout<<"Repository "<<repoFQN<<" does not exist."<<endl;
    return;
  }
  if (prNumber>targetrepo->numPrs){
    cout << "Invalid pull request number for repository " << repoFQN << ": out of range." << endl;
    return;
  }
  PullRequest *pr = targetrepo->prs[prNumber - 1];
  Commit *targetHead = nullptr;
    if (pr->toBranch != nullptr) {
      targetHead = pr->toBranch->head;
    } else {
        targetHead = targetrepo->commits; 
    }
  int count = 0;
    Commit *curr = pr->fromBranch->head;
    while (curr != nullptr) {
        bool isShared = false;
        Commit *tc = targetHead;
        while (tc != nullptr) {
            // 通过比对时间戳和留言，来精准识别跨仓库的“相同历史”
            if (tc->timestamp == curr->timestamp && strcmp(tc->message, curr->message) == 0) { 
                isShared = true; 
                break; 
            }
            tc = tc->next;
        }
        if (isShared) break; 
        
        count++;
        curr = curr->next;
    }

    Commit **uniqueCommits = nullptr;
    if (count > 0) {
        uniqueCommits = new Commit*[count];
        curr = pr->fromBranch->head;
        for (int i = 0; i < count; i++) {
            uniqueCommits[i] = curr;
            curr = curr->next;
        }
    }

    // ==========================================
    // 阶段 4：倒序回放（Replay）并重新计算 Hash
    // ==========================================
    Commit *hook = targetHead; // 就像挂钩一样，用来逐个接驳新车厢
    
    // 倒着遍历数组：从最老的 (count - 1) 开始，一路挂到最新的 (0)
    for (int i = count - 1; i >= 0; i--) {
        Commit *oldC = uniqueCommits[i];
        Commit *newC = new Commit;
        
        // 1. 拷贝原始属性，但替换为规定的 timestamp
        newC->author = oldC->author;
        strcpy(newC->message, oldC->message);
        newC->timestamp = timestamp; 
        
        // 2. 挂载到链表上
        newC->next = hook;
        newC->prev = nullptr;
        if (hook != nullptr) {
            hook->prev = newC;
        }
        
        // 3. 计算哈希 (利用自身的属性 + previous的属性)
        initialize(newC->hash);
        input(newC->hash, newC->author->name, strlen(newC->author->name));
        input(newC->hash, newC->message, strlen(newC->message));
        input(newC->hash, newC->timestamp);
        
        if (hook != nullptr) {
            input(newC->hash, hook->author->name, strlen(hook->author->name));
            input(newC->hash, hook->message, strlen(hook->message));
            input(newC->hash, hook->timestamp);
        }
        digest(newC->hash);
        
        // 把挂钩前移到刚建好的节点上，准备挂下一个
        hook = newC; 
    }
    
    // 用完数组后记得释放内存
    if (uniqueCommits != nullptr) {
        delete[] uniqueCommits;
    }

    // 4. 更新目标分支的 Head 指针，指向我们刚挂完的最新节点
    if (pr->toBranch != nullptr) {
        pr->toBranch->head = hook;
    } else {
        targetrepo->commits = hook;
    }

    // ==========================================
    // 阶段 5：安全销毁原分支及其独有 Commits (GC)
    // ==========================================
    // 这里使用了我们在 8.1 中打磨完美的终极安全销毁逻辑
    if (pr->fromBranch != nullptr) {
        Repository *fromRepo = pr->fromBranch->repo;
        Commit *curGC = pr->fromBranch->head;
        
        while (curGC != nullptr) {
            bool isShared = false;
            // 检查是否在主分支
            Commit *mc = fromRepo->commits;
            while (mc != nullptr) {
                if (curGC == mc) { isShared = true; break; }
                mc = mc->next;
            }
            
            // 检查是否在其他普通分支
            if (!isShared) {
                for (int i = 0; i < fromRepo->numBranches; i++) {
                    Branch *b = fromRepo->branches[i];
                    if (b == pr->fromBranch) continue;
                    
                    Commit *bc = b->head;
                    while (bc != nullptr) {
                        if (bc == curGC) { isShared = true; break; }
                        bc = bc->next;
                    }
                    if (isShared) break;
                }
            }
            
            if (isShared) break; // 遇到公共财产，停止销毁！
            
            // 安全删除节点
            Commit *toDelete = curGC;
            curGC = curGC->next;
            if (curGC != nullptr) curGC->prev = nullptr;
            delete toDelete;
        }

        // ==========================================
        // 阶段 6：从 Repository 移除 Branch 对象
        // ==========================================
        int bIdx = -1;
        for (int i = 0; i < fromRepo->numBranches; i++) {
            if (fromRepo->branches[i] == pr->fromBranch) {
                bIdx = i;
                break;
            }
        }
        
        if (bIdx != -1) {
            delete fromRepo->branches[bIdx]; // 释放这个分支的结构体
            
            for (int i = bIdx; i < fromRepo->numBranches - 1; i++) {
                fromRepo->branches[i] = fromRepo->branches[i + 1];
            }
            fromRepo->numBranches--;
            
            if (fromRepo->numBranches == 0) {
                delete[] fromRepo->branches;
                fromRepo->branches = nullptr;
            } else {
                Branch **newBranches = new Branch*[fromRepo->numBranches];
                for (int i = 0; i < fromRepo->numBranches; i++) {
                    newBranches[i] = fromRepo->branches[i];
                }
                delete[] fromRepo->branches;
                fromRepo->branches = newBranches;
            }
        }
        pr->fromBranch = nullptr; 
    }

    // ==========================================
    // 阶段 7：更新状态并打印成功
    // ==========================================
    pr->status = MERGED; 

    // 注意这里输出要求是 using rebase merge
    cout << "Pull request #" << prNumber << " in " << repoFQN 
         << " has been merged using rebase merge." << endl;
}

/**
 * Task 8.3 - Merge Pull Request (Merge Commit)
 *
 * The `merge_pull_request_mergecommit` function merges the specified
 * pull request in a repsitory using the merge commit strategy
 * (adds all commits to the target branch preserving chronological order, with a
 * final merge commit added).
 *
 * This is slightly different from what actually happens when a merge commit
 * is used, but for simplicity's sake this is done instead.
 *
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositories.
 * @param repoFQN: the name of the repository to merge a pull request for.
 * @param prNumber: the number of the pull request to merge.
 * @param timestamp: the timestamp when this pull request was merged.
 */
void merge_pull_request_mergecommit(RepositoryManagement &repoManagement,
                                    char *repoFQN, int prNumber,
                                    time_t timestamp) {
  // TODO: Task 8.3
  Repository *targetrepo = nullptr;
  for (int i=0; i<repoManagement.numRepos;i++){
    Repository *currepo=repoManagement.repos[i];
    char tempFQN[MAX_USER_NAME_LEN + MAX_REPO_NAME_LEN + 2];
    strcpy(tempFQN, currepo->owner->name);
    strcat(tempFQN, "/");
    strcat(tempFQN, currepo->name);
    if (strcmp(tempFQN,repoFQN)==0){
      targetrepo=currepo;
      break;
    }
  }
  if (targetrepo==nullptr){
    cout<<"Repository "<<repoFQN<<" does not exist."<<endl;
    return;
  }
  if (prNumber>targetrepo->numPrs){
    cout << "Invalid pull request number for repository " << repoFQN << ": out of range." << endl;
    return;
  }
  PullRequest *pr = targetrepo->prs[prNumber - 1];
  // ==========================================
    // 2. 确定目标分支当前最新的提交 (挂载点)
    // ==========================================
    Commit *targetHead = (pr->toBranch != nullptr) ? pr->toBranch->head : targetrepo->commits;

    // ==========================================
    // 3. 深拷贝源分支中的“独有提交”
    // ==========================================
    // 我们需要把 fromBranch 里的私有节点找出来，并倒序排列（因为链表头是最新的）
    int count = 0;
    Commit *curr = pr->fromBranch->head;
    while (curr != nullptr) {
        bool isShared = false;
        Commit *tc = targetHead;
        while (tc != nullptr) {
            // 通过比对时间戳和留言，来精准识别跨仓库的“相同历史”
            if (tc->timestamp == curr->timestamp && strcmp(tc->message, curr->message) == 0) { 
                isShared = true; 
                break; 
            }
            tc = tc->next;
        }
        if (isShared) break;
        count++;
        curr = curr->next;
    }

    // 依然采用“数组倒序回放法”，这样能保持时间顺序
    Commit **uniqueCommits = (count > 0) ? new Commit*[count] : nullptr;
    curr = pr->fromBranch->head;
    for (int i = 0; i < count; i++) {
        uniqueCommits[i] = curr;
        curr = curr->next;
    }

    // 依次拷贝并链接
    Commit *hook = targetHead;
    for (int i = count - 1; i >= 0; i--) {
        Commit *oldC = uniqueCommits[i];
        Commit *newC = new Commit;
        *newC = *oldC; // 复制所有内容 (含作者、信息、原始哈希、时间戳)
        
        // 重新构建链表关系
        newC->next = hook;
        newC->prev = nullptr;
        if (hook != nullptr) hook->prev = newC;
        
        hook = newC; // 挂钩前移
    }
    if (uniqueCommits) delete[] uniqueCommits;

    // ==========================================
    // 4. 创建最后的 Merge Commit (合并节点)
    // ==========================================
    Commit *finalMergeCommit = new Commit;
    finalMergeCommit->author = pr->author;
    finalMergeCommit->timestamp = timestamp;
    
    // 生成格式：Merge pull request #<number> from branch <branch>
    // 注意：这里的 <branch> 是 fromBranchFQN
    char mergeMsg[256];
    char fromFQN[256];
    // 按照 <Owner>/<Repo>:<Branch> 的格式拼装出 FQN
    sprintf(fromFQN, "%s/%s:%s", pr->fromBranch->repo->owner->name, 
                               pr->fromBranch->repo->name, 
                               pr->fromBranch->name);
    sprintf(mergeMsg, "Merge pull request #%d from branch %s", prNumber, fromFQN);
    strcpy(finalMergeCommit->message, mergeMsg);
    // 链接到刚才生成的链表最顶端
    finalMergeCommit->next = hook;
    finalMergeCommit->prev = nullptr;
    if (hook != nullptr) hook->prev = finalMergeCommit;

    // 计算合并提交的哈希值
    initialize(finalMergeCommit->hash);
    input(finalMergeCommit->hash, finalMergeCommit->author->name, strlen(finalMergeCommit->author->name));
    input(finalMergeCommit->hash, finalMergeCommit->message, strlen(finalMergeCommit->message));
    input(finalMergeCommit->hash, finalMergeCommit->timestamp);
    if (hook != nullptr) {
        input(finalMergeCommit->hash, hook->author->name, strlen(hook->author->name));
        input(finalMergeCommit->hash, hook->message, strlen(hook->message));
        input(finalMergeCommit->hash, hook->timestamp);
    }
    digest(finalMergeCommit->hash);

    // 更新目标分支的 Head
    if (pr->toBranch != nullptr) pr->toBranch->head = finalMergeCommit;
    else targetrepo->commits = finalMergeCommit;

    // ==========================================
    // 5. 垃圾回收：销毁源分支 (与 8.1 逻辑一致)
    // ==========================================
    if (pr->fromBranch != nullptr) {
        Repository *fromRepo = pr->fromBranch->repo;
        Commit *gcCurr = pr->fromBranch->head;
        while (gcCurr != nullptr) {
            bool isShared = false;
            Commit *mc = fromRepo->commits;
            while (mc != nullptr) {
                if (gcCurr == mc) { isShared = true; break; }
                mc = mc->next;
            }
            if (!isShared) {
                for (int i = 0; i < fromRepo->numBranches; i++) {
                    if (fromRepo->branches[i] == pr->fromBranch) continue;
                    Commit *bc = fromRepo->branches[i]->head;
                    while (bc != nullptr) {
                        if (gcCurr == bc) { isShared = true; break; }
                        bc = bc->next;
                    }
                    if (isShared) break;
                }
            }
            if (isShared) break;
            Commit *toDel = gcCurr;
            gcCurr = gcCurr->next;
            if (gcCurr) gcCurr->prev = nullptr;
            delete toDel;
        }

        // 从数组中删除 Branch 对象
        int bIdx = -1;
        for (int i = 0; i < fromRepo->numBranches; i++) {
            if (fromRepo->branches[i] == pr->fromBranch) { bIdx = i; break; }
        }
        if (bIdx != -1) {
            delete fromRepo->branches[bIdx];
            for (int i = bIdx; i < fromRepo->numBranches - 1; i++) fromRepo->branches[i] = fromRepo->branches[i+1];
            fromRepo->numBranches--;
            // 此处可选择缩容，逻辑同 8.1
        }
        pr->fromBranch = nullptr;
    }

    // ==========================================
    // 6. 更新状态与输出
    // ==========================================
    pr->status = MERGED;
    cout << "Pull request #" << prNumber << " in " << repoFQN 
         << " has been merged using a merge commit." << endl;
}

/**
 * Task 9 - De-register User
 *
 * The `deregister_user` function de-registers a user from the version control
 * system and updates any references to it with the ghost user. This deletes all
 * their repositories and then deallocates memory allocated to this user.
 *
 * @param userManagement: reference to the `UserManagement` structure containing
 *                        the linked list of users.
 * @param repoManagement: reference to the `RepositoryManagement` structure
 *                        containing the dynamic array of pointers to
 *                        repositopries.
 * @param username: the name of the user to de-register.
 */
void deregister_user(UserManagement &userManagement,
                     RepositoryManagement &repoManagement,
                     const char *username) {
  // TODO: Task 9
  // 1. 查找目标用户和 Ghost 用户
    User *targetUser = nullptr;
    User *ghostUser = nullptr;
    User *prevUser = nullptr; // 用于链表删除
    User *curr = userManagement.head;
    User *tempPrev = nullptr;

    while (curr != nullptr) {
        if (strcmp(curr->name, username) == 0) {
            targetUser = curr;
            prevUser = tempPrev;
        }
        if (strcmp(curr->name, "ghost") == 0) {
            ghostUser = curr;
        }
        tempPrev = curr;
        curr = curr->next;
    }

    if (targetUser == nullptr) {
        cout << "User " << username << " does not exist." << endl;
        return;
    }
    if (ghostUser == nullptr) {
        ghostUser = new User;
        strcpy(ghostUser->name, "ghost");
        ghostUser->numRepos = 0;
        ghostUser->repos = nullptr;
        ghostUser->next = userManagement.head;
        userManagement.head = ghostUser;
    }

    // 2. 全局遗迹移交 (Commits, PRs, Branches)
    // 即使该用户要走了，他留下的代码和记录也要改名为 "ghost" 继续存在
    for (int i = 0; i < repoManagement.numRepos; i++) {
        Repository *repo = repoManagement.repos[i];

        // 处理 Commits 链表
        Commit *c = repo->commits;
        while (c != nullptr) {
            if (c->author == targetUser) c->author = ghostUser;
            c = c->next;
        }

        // 处理 Pull Requests
        for (int j = 0; j < repo->numBranches; j++) {
            Commit *bc = repo->branches[j]->head;
            while (bc != nullptr) {
                if (bc->author == targetUser) bc->author = ghostUser;
                bc = bc->next;
            }
            if (repo->branches[j]->creator == targetUser) repo->branches[j]->creator = ghostUser;
        }

        // 处理 Branches
        for (int j = 0; j < repo->numBranches; j++) {
            if (repo->branches[j]->creator == targetUser) repo->branches[j]->creator = ghostUser;
        }
    }

    // 3. 销毁用户拥有的所有仓库
    // 注意：我们要从后往前删，或者先记录指针，避免数组位移导致的索引混乱
    for (int i = 0; i < targetUser->numRepos; i++) {
        Repository *repoToDelete = targetUser->repos[i];

        // A. 从全局 repoManagement 中移除
        int globalIdx = -1;
        for (int j = 0; j < repoManagement.numRepos; j++) {
            if (repoManagement.repos[j] == repoToDelete) {
                globalIdx = j;
                break;
            }
        }
        if (globalIdx != -1) {
            for (int j = globalIdx; j < repoManagement.numRepos - 1; j++) {
                repoManagement.repos[j] = repoManagement.repos[j+1];
            }
            repoManagement.numRepos--;
        }
        // 新增：从其他仓库的 Forks 数组中清除僵尸指针！
        for (int r = 0; r < repoManagement.numRepos; r++) {
            Repository *otherRepo = repoManagement.repos[r];
            int fIdx = -1;
            for (int f = 0; f < otherRepo->numForks; f++) {
                if (otherRepo->forks[f] == repoToDelete) { fIdx = f; break; }
            }
            if (fIdx != -1) {
                for (int f = fIdx; f < otherRepo->numForks - 1; f++) otherRepo->forks[f] = otherRepo->forks[f+1];
                otherRepo->numForks--;
            }
        }

        // B. 释放仓库深层内存 (由于 Task 1-8 可能已经实现了部分清理，这里需彻底)
        // 释放 Commits
        Commit *c = repoToDelete->commits;
        while (c != nullptr) {
            Commit *nextC = c->next;
            delete c;
            c = nextC;
        }
        // 释放 Branches
        for (int j = 0; j < repoToDelete->numBranches; j++) delete repoToDelete->branches[j];
        delete[] repoToDelete->branches;
        // 释放 PRs
        for (int j = 0; j < repoToDelete->numPrs; j++) delete repoToDelete->prs[j];
        delete[] repoToDelete->prs;
        // 释放 Forks 数组 (指针指向的 repo 会在它们各自 owner 注销时处理)
        delete[] repoToDelete->forks;

        delete repoToDelete;
    }
    delete[] targetUser->repos;

    // 4. 从 UserManagement 链表中移除用户
    if (prevUser == nullptr) {
        userManagement.head = targetUser->next;
    } else {
        prevUser->next = targetUser->next;
    }
    
    delete targetUser;

    cout << "User " << username << " has been deregistered." << endl;
}
