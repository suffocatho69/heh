document.addEventListener('DOMContentLoaded', function() {
    if (typeof recaptchav3_site_key === 'undefined') {
        return;
    }

    function handleFormSubmission(form, actionName) {
        form.addEventListener('submit', function(e) {
            // Check if we are already submitting with a fresh token
            if (form.dataset.recaptchaSubmitting === 'true') {
                return;
            }

            e.preventDefault();

            grecaptcha.ready(function() {
                grecaptcha.execute(recaptchav3_site_key, {action: actionName}).then(function(token) {
                    // Remove old token if exists
                    var oldInput = form.querySelector('input[name="recaptcha_token"]');
                    if (oldInput) {
                        oldInput.remove();
                    }

                    var hiddenInput = document.createElement('input');
                    hiddenInput.setAttribute('type', 'hidden');
                    hiddenInput.setAttribute('name', 'recaptcha_token');
                    hiddenInput.setAttribute('value', token);
                    form.appendChild(hiddenInput);

                    // Mark form as submitting to avoid recursion
                    form.dataset.recaptchaSubmitting = 'true';
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
        if (registerForm) {
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
