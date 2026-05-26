let getSkin = -1;

/* =========================
   INIT
========================= */

Cef.sendEvent("login:player_status", "");
Cef.sendEvent("login:name", "");

Cef.registerEventCallback("login:player_status", "OnPlayerStatus");
Cef.registerEventCallback("error:msg", "OnErrorMsg");
Cef.registerEventCallback("login:accept", "OnLoginAccept");
Cef.registerEventCallback("login:name", "OnLoginName");
Cef.registerEventCallback("pwd:login_succes", "OnLoginSuccess");

/* =========================
   CALLBACKS
========================= */

function OnPlayerStatus(response)
{
    if (parseInt(response) === 1)
        showLogin();
    else
        showReg();
}

function OnErrorMsg(response)
{
    showError(response);
}

function OnLoginAccept(response)
{
    if (parseInt(response) === 1)
    {
        resetWindow();

        document.getElementById('bodyd').style.backgroundColor = '#000';
        document.getElementById('bodyd').style.transition = '0.5s';

        setTimeout(screenDimming, 4000);
    }
}

function OnLoginName(response)
{
    const regLogin = document.getElementById('reg-login');
    regLogin.value = response;
    regLogin.readOnly = true;

    const logLogin = document.getElementById('log-login');
    logLogin.value = response;
    logLogin.readOnly = true;
}

function OnLoginSuccess(response)
{
    if (parseInt(response) === 1)
    {
        resetWindow();

        Cef.sendEvent("pwd:exit_forms", "");
    }
    else
    {
        resetWindow();
    }
}

/* =========================
   WINDOW CONTROL
========================= */

function showReg()
{
    resetError();

    document.getElementById('login').style.display = 'none';
    document.getElementById('register').style.display = 'block';

    document.getElementById('cc-selector').style.display = 'none';
    document.getElementById('cc-selector-fem').style.display = 'none';
}

function showLogin()
{
    resetError();

    document.getElementById('login').style.display = 'block';
    document.getElementById('register').style.display = 'none';

    document.getElementById('cc-selector').style.display = 'none';
    document.getElementById('cc-selector-fem').style.display = 'none';
}

function resetWindow()
{
    document.getElementById('login').style.display = 'none';
    document.getElementById('register').style.display = 'none';

    document.getElementById('cc-selector').style.display = 'none';
    document.getElementById('cc-selector-fem').style.display = 'none';

    document.getElementById('windowr').style.display = 'none';
    document.getElementById('radios').style.display = 'none';

    document.getElementById('error').style.display = 'none';

    document.getElementsByClassName('form')[0].style.display = 'none';
}

function screenDimming()
{
    document.getElementById('bodyd').style.backgroundColor = '';
    document.getElementById('bodyd').style.transition = '2s';
}

/* =========================
   LOGIN
========================= */

function loginAttempt()
{
    const login = document.getElementById('log-login').value.trim();
    const password = document.getElementById('log-password').value.trim();

    resetError();

    if (!password || password.length < 6)
        return showError('Введите Пароль');

    let attempt = login + ',' + password;

    Cef.sendEvent('pwd:try', attempt);
}

/* =========================
   REGISTER
========================= */

function registerAttempt()
{
    const login = document.getElementById('reg-login').value.trim();
    const mail = document.getElementById('reg-mail').value.trim();

    const password = document.getElementById('reg-password').value.trim();
    const passwordConfirm = document.getElementById('reg-password-confirm').value.trim();

    const genderFemale = document.getElementById('female');
    const genderMale = document.getElementById('male');

    resetError();

    let mailRegex = /^([A-Za-z0-9_\-\.])+\@([A-Za-z0-9_\-\.])+\.([A-Za-z]{2,4})$/;

    if (!mail || mail.length < 3 || !mailRegex.test(mail))
        return showError('Введите корректный email');

    let passwordRegex = /^[A-Za-z0-9]{6,18}$/;

    if (!passwordRegex.test(password))
    {
        return showError(
            'Пароль может состоять только из латинских букв и цифр (от 6 до 18 символов)'
        );
    }

    if (password !== passwordConfirm)
        return showError('Пароли не совпадают');

    if (!genderMale.checked && !genderFemale.checked)
        return showError('Выбери пол персонажа');

    const skins = [6, 22, 48, 56, 69, 41];

    if (getSkin < 1 || getSkin > skins.length)
        return showError('Выбери скин персонажа');

    const gender = genderMale.checked ? 2 : 1;

    let attempt =
        login + ',' +
        password + ',' +
        mail + ',' +
        gender + ',' +
        skins[getSkin - 1];

    Cef.sendEvent('pwd:reg', attempt);

    resetWindow();

    document.getElementById('bodyd').style.backgroundColor = '#000';
    document.getElementById('bodyd').style.transition = '0.5s';

    setTimeout(screenDimming, 4000);
}

/* =========================
   EXIT
========================= */

function onExitClick()
{
    Cef.sendEvent('pwd:exit_forms', '');
}

/* =========================
   ERROR
========================= */

function showError(message)
{
    const errorBlock = document.getElementById('error');

    errorBlock.innerText = message;
    errorBlock.style.display = 'block';

    const errorBlock2 = document.getElementById('error_reg');

    errorBlock2.innerText = message;
    errorBlock2.style.display = 'block';
}

function resetError()
{
    const errorBlock = document.getElementById('error');

    errorBlock.innerText = '';
    errorBlock.style.display = 'none';

    const errorBlock2 = document.getElementById('error_reg');

    errorBlock2.innerText = '';
    errorBlock2.style.display = 'none';
}

/* =========================
   GENDER / SKIN
========================= */

function clickgender(res)
{
    if (parseInt(res) === 1)
    {
        document.querySelector('#cc-selector-fem').style.display = 'none';
        document.querySelector('#cc-selector').style.display = 'block';
    }
    else
    {
        document.querySelector('#cc-selector-fem').style.display = 'block';
        document.querySelector('#cc-selector').style.display = 'none';
    }
}

function isCheckedSkinGender(res)
{
    getSkin = parseInt(res);
					  }
