document.addEventListener('DOMContentLoaded', function() {
    if (typeof recaptchav3_site_key === 'undefined') {
        return;
    }

    function handleFormSubmission(form, actionName) {
        form.addEventListener('submit', function(e) {
            if (form.querySelector('input[name="recaptcha_token"]')) {
                return; // Token already added and form is being submitted
            }

            e.preventDefault();

            grecaptcha.ready(function() {
                grecaptcha.execute(recaptchav3_site_key, {action: actionName}).then(function(token) {
                    var hiddenInput = document.createElement('input');
                    hiddenInput.setAttribute('type', 'hidden');
                    hiddenInput.setAttribute('name', 'recaptcha_token');
                    hiddenInput.setAttribute('value', token);
                    form.appendChild(hiddenInput);

                    // Re-submit the form
                    form.submit();
                });
            });
        });
    }

    // Contact Form
    if (recaptchav3_enable_contact) {
        var contactForm = document.querySelector('#contact-form');
        if (contactForm) {
            handleFormSubmission(contactForm, 'contact');
        }
    }

    // Registration Form
    if (recaptchav3_enable_register) {
        var registerForm = document.querySelector('#customer-form');
        if (registerForm && registerForm.action.indexOf('create_account') !== -1) {
             handleFormSubmission(registerForm, 'register');
        }
    }

    // Login Form
    if (recaptchav3_enable_login) {
        var loginForm = document.querySelector('#login-form');
        if (loginForm) {
            handleFormSubmission(loginForm, 'login');
        }
    }
});
