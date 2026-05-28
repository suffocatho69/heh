document.addEventListener('DOMContentLoaded', function() {
    if (typeof recaptchav3_site_key === 'undefined') {
        return;
    }

    function addTokenToForm(form, token) {
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
                    addTokenToForm(form, token);

                    // Mark form as submitting to avoid recursion
                    form.dataset.recaptchaSubmitting = 'true';
                    form.submit();
                });
            });
        });
    }

    // Contact Form
    if (typeof recaptchav3_enable_contact !== 'undefined' && recaptchav3_enable_contact) {
        var contactForm = document.querySelector('#contact-form');
        if (contactForm) {
            handleFormSubmission(contactForm, 'contact');
        }
    }

    // Registration Form
    if (typeof recaptchav3_enable_register !== 'undefined' && recaptchav3_enable_register) {
        var registerForm = document.querySelector('#customer-form');
        if (registerForm) {
             handleFormSubmission(registerForm, 'register');
        }
    }

    // Login Form
    if (typeof recaptchav3_enable_login !== 'undefined' && recaptchav3_enable_login) {
        var loginForm = document.querySelector('#login-form');
        if (loginForm) {
            handleFormSubmission(loginForm, 'login');
        }
    }

    // Order Process
    if (typeof recaptchav3_enable_order !== 'undefined' && recaptchav3_enable_order) {
        // We need to handle the "Order with obligation to pay" button
        // and potentially other steps in the checkout.

        // Final payment confirmation
        var paymentConfirmation = document.querySelector('#payment-confirmation button');
        if (paymentConfirmation) {
            paymentConfirmation.addEventListener('click', function(e) {
                var btn = e.currentTarget;
                if (btn.dataset.recaptchaVerified === 'true') {
                    return;
                }

                e.preventDefault();
                e.stopPropagation();

                grecaptcha.ready(function() {
                    grecaptcha.execute(recaptchav3_site_key, {action: 'order'}).then(function(token) {
                        // In PS 1.7/8 checkout, the payment button is often outside the actual payment form
                        // or triggers an action that we need to intercept.
                        // We'll try to find the active payment form.
                        var activePaymentForm = document.querySelector('.payment-options .active form');
                        if (activePaymentForm) {
                            addTokenToForm(activePaymentForm, token);
                        } else {
                            // Fallback: append to any visible payment form or create a hidden one if needed
                            // For standard checkout, there's usually one form per payment option.
                            var forms = document.querySelectorAll('.payment-option form');
                            forms.forEach(function(f) {
                                addTokenToForm(f, token);
                            });
                        }

                        btn.dataset.recaptchaVerified = 'true';
                        btn.click();
                    });
                });
            }, true); // Use capture to ensure we run before other handlers
        }

        // Also handle step confirmations (addresses, delivery)
        var stepButtons = [
            'button[name="confirm-addresses"]',
            'button[name="confirmDeliveryOption"]'
        ];

        stepButtons.forEach(function(selector) {
            var btn = document.querySelector(selector);
            if (btn) {
                btn.addEventListener('click', function(e) {
                    var currentBtn = e.currentTarget;
                    if (currentBtn.dataset.recaptchaVerified === 'true') {
                        return;
                    }

                    e.preventDefault();
                    e.stopPropagation();

                    grecaptcha.ready(function() {
                        grecaptcha.execute(recaptchav3_site_key, {action: 'order'}).then(function(token) {
                            var form = currentBtn.closest('form');
                            if (form) {
                                addTokenToForm(form, token);
                            }
                            currentBtn.dataset.recaptchaVerified = 'true';
                            currentBtn.click();
                        });
                    });
                }, true);
            }
        });
    }
});
