# Tired of typing in passwords? Setup ssh authentication using private/public keys

You can refer to this anytime in your life–the day you get tired from typing in passwords.

## Windows Users

1. Download and install Git bash [https://gitforwindows.org](https://gitforwindows.org/). All the following steps should be ran in `git bash`. Yay Windows!
2. Go into your home directory `cd ~`
3. Run the following command

```bash
ssh-keygen -t rsa -b 4096
```

1. Press enter when the you see the following prompt:

```bash
Enter file in which to save the key (C:\Users\<username>/.ssh/id_rsa):
```

1. Scenarios. You will get counter one of the 2 scenarios:
    1. In case you get a warning below, this means that you already have a public/private key pair. Most likely you do **not** want to overwrite the existing key pair, so just type `n` followed by enter. (If you want to overwrite the existing key pair, just type `y` followed by enter)
        
        ```bash
        C:\Users\<username>/.ssh/id_rsa already exists.
        Overwrite (y/n)?
        ```
        
    2. In case you see the output below, just press enter and enter again:
        
        ```bash
        Enter passphrase (empty for no passphrase):
        ```
        
        You know everything was successful when you see the a message similar to this (will have some differences):
        
        ```bash
        Your identification has been saved in C:\Users\<username>/.ssh/id_rsa
        Your public key has been saved in C:\Users\<username>/.ssh/id_rsa.pub
        The key fingerprint is:
        SHA256:RIBfYniQPNmfgXiaPdpLYsSVVKkJ/F5v2yLqAacqMZI kinematic_engine24@DESKTOP-D2HK96O
        The key's randomart image is:
        +---[RSA 4096]----+
        |   o.Xo=o.       |
        |    @ X.+        |
        |   . & *.o       |
        |    = B.+        |
        | . ..+.oS.       |
        |E   +++   o      |
        |.o ..o.. . o     |
        |.  .  ... o .    |
        | ..  .o. . .     |
        +----[SHA256]-----+
        ```
        
2. At this point, you. should now have these 2 files, `id_rsa` and `id_rsa.pub` in `C:\Users\<username>/.ssh`
3. Now, we need to transfer the public key to the machine you will be ssh’ing into. First navigate to `.ssh` directory in your home directory
    
    ```bash
    cd ~/.ssh
    ```
    
4. Use `ssh-copy-id` to copy the public key to whatever device you’re ssh’ing to: 
    
    `ssh-copy-id username@remote`
    
    If the port number is not 22, you can add it (for example, ssh is on port 2200):
    
    `ssh-copy-id -p 2200 username@remote`
    
5. Congrats now test it. you don't need password anymore.


<aside>
⚠️ Never share `id_rsa` with anyone! If this file is leaked, create a new public/private key pair and replace your old public key with your new public on all the machines you remote into.
You can freely share `id_rsa.pub`

</aside>

## Mac and Linux Users

1. Go into your home directory `cd ~`
2. Run the following command

```bash
ssh-keygen -t rsa -b 4096
```

1. Press enter when the you see the following prompt:

```bash
Enter file in which to save the key (/Users/<username>/.ssh/.ssh/id_rsa):
```

1. Scenarios. You will get counter one of the 2 scenarios:
    1. In case you get a warning below, this means that you already have a public/private key pair. Most likely you do **not** want to overwrite the existing key pair, so just type `n` followed by enter. (If you want to overwrite the existing key pair, just type `y` followed by enter)
        
        ```bash
        /Users/<username>/.ssh/id_rsa already exists.
        Overwrite (y/n)?
        ```
        
    2. In case you see the output below, just press enter and enter again:
        
        ```bash
        Enter passphrase (empty for no passphrase):
        ```
        
        You know everything was successful when you see the a message similar to this (will have some differences):
        
        ```bash
        Your identification has been saved in /Users/<username>/.ssh/id_rsa
        Your public key has been saved in /Users/<username>/.ssh/id_rsa.pub
        The key fingerprint is:
        SHA256:RIBfYniQPNmfgXiaPdpLYsSVVKkJ/F5v2yLqAacqMZI kinematic_engine24@DESKTOP-D2HK96O
        The key's randomart image is:
        +---[RSA 4096]----+
        |   o.Xo=o.       |
        |    @ X.+        |
        |   . & *.o       |
        |    = B.+        |
        | . ..+.oS.       |
        |E   +++   o      |
        |.o ..o.. . o     |
        |.  .  ... o .    |
        | ..  .o. . .     |
        +----[SHA256]-----+
        ```
        
2. At this point, you. should now have these 2 files, `id_rsa` and `id_rsa.pub` in `/Users/<username>/.ssh`
3. Now, we need to transfer the public key to the machine you will be ssh’ing into. First navigate to `.ssh` directory in your home directory
    
    ```bash
    cd ~/.ssh
    ```
    
4. Use `ssh-copy-id` to copy the public key to whatever device you’re ssh’ing to: 
    
    `ssh-copy-id username@remote`
    
    If the port number is not 22, you can add it (for example, ssh is on port 2200):
    
    `ssh-copy-id -p 2200 username@remote`
    
5. Congrats


<aside>
⚠️ Never share `id_rsa` with anyone! If this file is leaked, create a new public/private key pair and replace your old public key with your new public on all the machines you remote into.
You can freely share `id_rsa.pub`

</aside>