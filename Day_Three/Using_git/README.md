# Git/GitHub Homework

For this presentation we will be doing a hands on tutorial. To make this tutorial run 
smoothly, please complete the following steps prior to day 3. 


1. Create a free account at: https://github.com/signup
    - Remember the email you used!

2. SSH into a login node:
```
ssh <username>@login.rc.colorado.edu
```

3. Add your git user name
```
git config --global user.name <github user name>
```

4. Add your git user email
```
git config --global user.email <github account email>
```

5. Check that everything was set (should display user.name and user.email)
```
git config --list
```

6. Create your ssh key
```
ssh-keygen -t ed25519 
```
- Press enter for all prompts (accepts default options)
- This will put the ssh key in `~/.ssh/`

7. Display your public key so we can provide it to GitHub:
```
cat ~/.ssh/id_ed25519.pub
```

8. Go to [github.com](https://github.com/) and follow this procedure:
    - Login, if you are not
    - Click on the drop-down menu on your profile icon located in the top right corner
    - Select "Settings" from the drop-down menu
    - On the left hand side under the "Access" section select "SSH and GPG keys"
    - Click the green box that says "New SSH key"
    - Add a title (e.g. my_rc_key)
    - Leave "Key type" as "Authentication Key"
    - In the "Key" box paste your ssh key provided by step 7. You do not need to include 
the end portion "<username>@loginXX"
    - Select "Add SSH key"
   
9.  Add the following to `~/.ssh/config`:
```
Host github.com
    Hostname github.com
    IdentityFile ~/.ssh/id_ed25519
    IdentitiesOnly yes
```

- You should see the message: "Hi <username>! You've successfully authenticated, but GitHub does not provide shell access."

